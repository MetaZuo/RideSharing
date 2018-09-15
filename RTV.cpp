#ifndef _RTV_CPP_
#define _RTV_CPP_

#include <map>
#include <algorithm>
#include <string>
#include <ctime>
#include "globals.cpp"
#include "RV.cpp"
#include "gurobi_c++.h"
using namespace std;


bool equal_to_sub(vector<int>& compared, vector<int>& origin, int excludeIdx) {
    vector<int>::iterator iterCompared = compared.begin();
    vector<int>::iterator iterOrigin = origin.begin();
    int originIdx = 0;
    while (iterCompared != compared.end() && iterOrigin != origin.end()) {
        if (originIdx == excludeIdx) {
            iterOrigin++;
            originIdx++;
            continue;
        }
        if (*iterCompared != *iterOrigin) {
            return false;
        }
        iterCompared++;
        iterOrigin++;
        originIdx++;
    }
    return true;
}

class RTVGraph {

    int numRequests, numTrips, numVehicles;

    //---- these are nodes of RTV graph ----
    vector<vector<int> > trips; // trips[tIdx] = trip
    vector<int> vIds; // vIds[vIdx] = vehicleId
    //--------------------------------------

    map<vector<int>, int> trip_tIdx; // trip -> tIdx, reverse of vector "trips"

    class TIdxComparable {
    public:
        static RTVGraph *rtvGraph;
        int tIdx;

        TIdxComparable(int tIdx) : tIdx(tIdx) {}

        bool operator < (const TIdxComparable& other) const {
            int size1 = rtvGraph->trips[tIdx].size();
            int size2 = rtvGraph->trips[other.tIdx].size();
            if (size1 > size2) {
                return true;
            } else if (size1 == size2) {
                return tIdx < other.tIdx;
            } else {
                return false;
            }
        }
    };

    //---- these are edges of RTV graph ----
    map<int, set<int> > rId_tIdxes;
    // trip -> edges to vehicles (<cost, vIdx> pairs)
    map<TIdxComparable, vector<pair<int, int> > > tIdx_vCostIdxes;
    vector<vector<int> > vIdx_tIdxes;
    //--------------------------------------

    int addVehicleId(int vehicleId) {
        vIds.push_back(vehicleId);
        vIdx_tIdxes.push_back(vector<int>());
        return numVehicles++;
    }

    int getTIdx(vector<int>& trip) {
        map<vector<int>, int>::iterator iter = trip_tIdx.find(trip);
        if (iter != trip_tIdx.end()) {
            return iter->second;
        }
        trip_tIdx[trip] = numTrips;
        trips.push_back(trip);
        vector<int>::iterator iterRIdx = trip.begin();
        while (iterRIdx != trip.end()) {
            int rId = *iterRIdx;
            if (rId_tIdxes.find(rId) == rId_tIdxes.end()) {
                rId_tIdxes[rId] = set<int>();
            }
            rId_tIdxes[rId].insert(numTrips);
            iterRIdx++;
        }
        return numTrips++;
    }

    void add_edge_trip_vehicle(int tIdx, pair<int, int> cost_vIdx) {
        TIdxComparable tIdxComparable(tIdx);
        if (tIdx_vCostIdxes.find(tIdxComparable) == tIdx_vCostIdxes.end()) {
            tIdx_vCostIdxes[tIdxComparable] = vector<pair<int, int> >();
        }
        tIdx_vCostIdxes[tIdxComparable].push_back(cost_vIdx);
        vIdx_tIdxes[cost_vIdx.second].push_back(tIdx);
    }

    void build_single_vehicle(int vehicleId, RVGraph *rvGraph,
    vector<Vehicle>& vehicles, vector<Request>& requests,
    map<pair<int, int>, int> *dist, double timeLimit) {

        clock_t beginClock = clock();

        Vehicle& vehicle = vehicles[vehicleId];
        int vIdx = addVehicleId(vehicleId);

        vector<int> tIdxListOfCapacity[max_capacity + 1];

        // Add trips of size 1
        if (max_capacity - vehicle.get_num_passengers() > 0) {
            tIdxListOfCapacity[1].reserve(requests.size());

            vector<pair<int, int> > edges;
            edges.reserve(requests.size());
            rvGraph->get_vehicle_edges(vehicleId, edges);

            vector<pair<int, int> >::iterator iter = edges.begin();
            while (iter != edges.end()) {
                int reqId = iter->first;
                int cost = iter->second;
                vector<int> trip(1);
                trip[0] = reqId;
                int tIdx = getTIdx(trip);
                tIdxListOfCapacity[1].push_back(tIdx);
                add_edge_trip_vehicle(tIdx, make_pair(cost, vIdx));
                iter++;
            }
        }

        if (max_capacity - vehicle.get_num_passengers() < 2) {
            return;
        }
        // Add trips of size 2
        int prevSize = tIdxListOfCapacity[1].size();
        tIdxListOfCapacity[2].reserve(prevSize * (prevSize - 1) / 2);
        vector<int> trip(2);
        for (int i = 0; i < tIdxListOfCapacity[1].size(); i++) {
            for (int j = i + 1; j < tIdxListOfCapacity[1].size(); j++) {
                int r1 = trips[tIdxListOfCapacity[1][i]][0];
                int r2 = trips[tIdxListOfCapacity[1][j]][0];
                if (rvGraph->has_reqs_edge(r1, r2)) {
                    Request *reqs[2] = {&requests[r1], &requests[r2]};
                    int cost = travel(vehicle, reqs, 2, dist, false);
                    if (cost >= 0) {
                        trip[0] = r1, trip[1] = r2;
                        int tIdx = getTIdx(trip);
                        tIdxListOfCapacity[2].push_back(tIdx);
                        add_edge_trip_vehicle(tIdx, make_pair(cost, vIdx));
                    }
                }
                /*
                clock_t nowClock = clock();
                if ((double(nowClock - beginClock)) / CLOCKS_PER_SEC > timeLimit) {
                    printf("TLE at size 2, time = %f\n", (double(nowClock - beginClock)) / CLOCKS_PER_SEC);
                    return;
                }
                 */
            }
        }

        Request *reqs[max_capacity];
        // Add trips of size k
        for (int k = 3; k <= max_capacity - vehicle.get_num_passengers(); k++) {
            prevSize = tIdxListOfCapacity[k - 1].size();
            tIdxListOfCapacity[k].reserve(prevSize * (prevSize - 1) / 2);
            for (int i = 0; i < tIdxListOfCapacity[k - 1].size(); i++) {
                for (int j = i + 1; j < tIdxListOfCapacity[k - 1].size(); j++) {
                    vector<int>& trip1 = trips[tIdxListOfCapacity[k - 1][i]];
                    vector<int>& trip2 = trips[tIdxListOfCapacity[k - 1][j]];
                    vector<int> trip;
                    set_union(trip1.begin(), trip1.end(), trip2.begin(), trip2.end(), back_inserter(trip));
                    if (trip2.size() != k) {
                        continue;
                    }
                    bool allSubsExist = true;
                    // exclude one element
                    for (int idx = 0; idx < k; idx++) {
                        bool subExist = false;
                        // for each existing trip of size k - 1
                        for (int sub = 0; sub < tIdxListOfCapacity[k - 1].size(); sub++) {
                            if (equal_to_sub(trips[tIdxListOfCapacity[k - 1][sub]], trip, idx)) {
                                subExist = true;
                                break;
                            }
                        }
                        if (!subExist) {
                            allSubsExist = false;
                            break;
                        }
                    }
                    if (!allSubsExist) {
                        continue;
                    }
                    for (int idx = 0; idx < k; idx++) {
                        reqs[idx] = &requests[trip[idx]];
                    }
                    int cost = travel(vehicle, reqs, k, dist, false);
                    if (cost >= 0) {
                        int tIdx = getTIdx(trip);
                        tIdxListOfCapacity[k].push_back(tIdx);
                        add_edge_trip_vehicle(tIdx, make_pair(cost, vIdx));
                    }
                    /*
                    clock_t nowClock = clock();
                    if ((double(nowClock - beginClock)) / CLOCKS_PER_SEC > timeLimit) {
                        printf("TLE at size %d, time = %f\n", k, (double(nowClock - beginClock)) / CLOCKS_PER_SEC);
                        return;
                    }
                     */
                }
            }
        }
    }

    void sort_edges() {
        map<TIdxComparable, vector<pair<int, int> > >::iterator iter
            = tIdx_vCostIdxes.begin();
        while (iter != tIdx_vCostIdxes.end()) {
            sort(iter->second.begin(), iter->second.end());
            iter++;
        }
    }

    void greedy_assign_same_trip_size(
        vector<vector<pair<int, int> >::iterator>& edgeIters,
        vector<vector<pair<int, int> >::iterator>& edgeEnds,
        vector<int>& tIdxes,
        set<int>& assignedRIds, set<int>& assignedVIdxes,
        GRBVar **epsilon, GRBVar *chi
    ) {
        int numEdgeVectors = edgeIters.size();
        int numNotEnded = numEdgeVectors;

        while (numNotEnded > 0) {
            // get minimal cost edge
            int minCost = 0x7fffffff, argMin = -1;
            for (int i = 0; i < numEdgeVectors; i++) {
                if (edgeIters[i] != edgeEnds[i]) {
                    int tmpCost = edgeIters[i]->first;
                    if (tmpCost < minCost) {
                        minCost = tmpCost;
                        argMin = i;
                    }
                }
            }

            int tIdx = tIdxes[argMin];
            int vIdx = edgeIters[argMin]->second;

            // check if the edge can be assigned
            vector<int>::iterator iterRId;
            bool allReqsUnassigned = true;
            for (iterRId = trips[tIdx].begin(); iterRId != trips[tIdx].end(); iterRId++) {
                if (assignedRIds.find(*iterRId) != assignedRIds.end()) {
                    allReqsUnassigned = false;
                    break;
                }
            }
            if (allReqsUnassigned && assignedVIdxes.find(vIdx) == assignedVIdxes.end()) {
                // assign the edge
                try {
                    epsilon[tIdx][vIdx].set(GRB_DoubleAttr_Start, 1.0);
                    for (iterRId = trips[tIdx].begin(); iterRId != trips[tIdx].end(); iterRId++) {
                        chi[*iterRId].set(GRB_DoubleAttr_Start, 0.0);
                        assignedRIds.insert(*iterRId);
                    }
                    assignedVIdxes.insert(vIdx);
                } catch (GRBException& e) {
                    printf("%s\n", e.getMessage().c_str());
                }
            }

            // move the argMin-th iterator
            edgeIters[argMin]++;
            if (edgeIters[argMin] == edgeEnds[argMin]) {
                numNotEnded--;
            }
        }
    }

public:
    RTVGraph(RVGraph *rvGraph, vector<Vehicle>& vehicles,
    vector<Request>& requests, map<pair<int, int>, int> *dist) {
        numRequests = requests.size();
        numTrips = 0;
        numVehicles = 0;
        TIdxComparable::rtvGraph = this;
        double timeLimit = double(time_step) * 0.8 / rvGraph->get_vehicle_num();
        printf("time limit = %f\n", timeLimit);
        for (int i = 0; i < vehicles.size(); i++) {
            if (rvGraph->has_vehicle(i)) {
                build_single_vehicle(i, rvGraph, vehicles, requests, dist, timeLimit);
            }
        }
        printf("begin to sort edges\n");
        sort_edges();
    }

    void solve(GRBEnv *env,
        vector<Vehicle>& vehicles, vector<Request>& requests,
        vector<Request>& unservedCollector,
        map<pair<int, int>, int> *dist) {
        
        printf("Defining variables...\n");
        GRBModel model = GRBModel(*env);
        GRBVar **epsilon = new GRBVar* [numTrips];
        for (int i = 0; i < numTrips; i++) {
            epsilon[i] = model.addVars(numVehicles, GRB_BINARY);
        }
        GRBVar *chi = model.addVars(numRequests, GRB_BINARY);

        // default initial values
        printf("Initializing...\n");
        for (int i = 0; i < numTrips; i++) {
            for (int j = 0; j < numVehicles; j++) {
                epsilon[i][j].set(GRB_DoubleAttr_Start, 0.0);
            }
        }
        for (int i = 0; i < numRequests; i++) {
            chi[i].set(GRB_DoubleAttr_Start, 1.0);
        }

        map<int, set<int> >::iterator iterRV;

        // add constraints
        printf("Adding constraint #1 ...\n");
        for (int vIdx = 0; vIdx < numVehicles; vIdx++) {
            GRBLinExpr constr = 0;
            vector<int>& tIdxes = vIdx_tIdxes[vIdx];
            vector<int>::iterator iter = tIdxes.begin();
            while (iter != tIdxes.end()) {
                constr += epsilon[*iter][vIdx];
                iter++;
            }
            model.addConstr(constr <= 1.0 + minimal);
        }
        printf("Adding constraint #2 ...\n");
        for (iterRV = rId_tIdxes.begin(); iterRV != rId_tIdxes.end(); iterRV++) {
            GRBLinExpr constr = 0;
            int rId = iterRV->first;
            set<int>::iterator iterTIdx = iterRV->second.begin();
            while (iterTIdx != iterRV->second.end()) {
                int tIdx = *iterTIdx;
                vector<pair<int, int> >& vCostIdxes
                    = tIdx_vCostIdxes[TIdxComparable(tIdx)];
                vector<pair<int, int> >::iterator iter = vCostIdxes.begin();
                while (iter != vCostIdxes.end()) {
                    int vIdx = iter->second;
                    constr += epsilon[tIdx][vIdx];
                    iter++;
                }
                iterTIdx++;
            }
            constr += chi[rId];
            model.addConstr(constr == 1.0);
            // model.addConstr(constr <= 1.0 + minimal);
        }

        // greedy assignment
        printf("Greedy assignment...\n");
        set<int> assignedRIds, assignedVIdxes;
        map<TIdxComparable, vector<pair<int, int> > >::iterator iterTV;
        vector<vector<pair<int, int> >::iterator> edgeIters, edgeEnds;
        vector<int> tIdxes;
        int tripSize = max_capacity + 1;
        /*
        printf("size of trips: %d %d\n", tIdx_vCostIdxes.size(), trips.size());
        for (iterTV = tIdx_vCostIdxes.begin(); iterTV != tIdx_vCostIdxes.end(); iterTV++) {
            printf("%d, ", trips[iterTV->first.tIdx].size());
        }
        printf("\n");
         */
        for (iterTV = tIdx_vCostIdxes.begin(); iterTV != tIdx_vCostIdxes.end(); iterTV++) {
            if (tripSize > trips[iterTV->first.tIdx].size()) {
                greedy_assign_same_trip_size(
                    edgeIters, edgeEnds, tIdxes, assignedRIds, assignedVIdxes,
                    epsilon, chi
                );
                tripSize = trips[iterTV->first.tIdx].size();
                edgeIters.clear();
                edgeEnds.clear();
                tIdxes.clear();
            }
            edgeIters.push_back(iterTV->second.begin());
            edgeEnds.push_back(iterTV->second.end());
            tIdxes.push_back(iterTV->first.tIdx);
        }
        greedy_assign_same_trip_size(
            edgeIters, edgeEnds, tIdxes, assignedRIds, assignedVIdxes,
            epsilon, chi
        );

        // build objective expression
        GRBLinExpr objective = 0;
        printf("Generating objective expression...\n");
        for (int tIdx = 0; tIdx < numTrips; tIdx++) {
            vector<pair<int, int> >& vCostIdxes
                = tIdx_vCostIdxes[TIdxComparable(tIdx)];
            vector<pair<int, int> >::iterator iter = vCostIdxes.begin();
            while (iter != vCostIdxes.end()) {
                int cost = iter->first;
                int vIdx = iter->second;
                objective += epsilon[tIdx][vIdx] * cost;
                iter++;
            }
        }
        for (iterRV = rId_tIdxes.begin(); iterRV != rId_tIdxes.end(); iterRV++) {
            objective += chi[iterRV->first] * penalty;
        }
        model.setObjective(objective, GRB_MINIMIZE);

        // solve
        printf("Solving...\n");
        model.optimize();

        printf("numRequests = %d\n", numRequests);
        printf("Assigned vehicle-trip pairs:\n");
        int cnt = 0;
        for (int vIdx = 0; vIdx < numVehicles; vIdx++) {
            for (int tIdx = 0; tIdx < numTrips; tIdx++) {
                double val = epsilon[tIdx][vIdx].get(GRB_DoubleAttr_X);
                if (val < 1.0 + minimal && val > 1.0 - minimal) {
                    printf("Vehicle #%d: [", vIds[vIdx]);
                    Vehicle& vehicle = vehicles[vIds[vIdx]];
                    Request *reqs[max_capacity];
                    int tripSize = 0;
                    vector<int>::iterator iter = trips[tIdx].begin();
                    printf("%d", requests[*iter].unique);
                    reqs[tripSize++] = &requests[*iter];
                    cnt++;
                    iter++;
                    while (iter != trips[tIdx].end()) {
                        printf(", %d", requests[*iter].unique);
                        reqs[tripSize++] = &requests[*iter];
                        cnt++;
                        iter++;
                    }
                    printf("]\n");

                    // update passengers of vehicle
                    travel(vehicle, reqs, tripSize, dist, true);

                    break;
                }
            }
        }
        printf("Number of served requests: %d\n\n", cnt);

        unservedCollector.clear();
        cnt = 0;
        printf("Requests not served: [");
        /*
        for (iterRV = rId_tIdxes.begin(); iterRV != rId_tIdxes.end(); iterRV++) {
            if (chi[iterRV->first].get(GRB_DoubleAttr_X) > 1.0 - minimal) {
                unservedCollector.push_back(requests[iterRV->first]);
                printf("%d, ", requests[iterRV->first].unique);
                cnt++;
            }
        }
         */
        for (int rId = 0; rId < numRequests; rId++) {
            if (rId_tIdxes.find(rId) == rId_tIdxes.end() ||
                chi[rId].get(GRB_DoubleAttr_X) > 1.0 - minimal) {
                unservedCollector.push_back(requests[rId]);
                printf("%d, ", requests[rId].unique);
                cnt++;
            }
        }
        printf("]\n");
        printf("Number of unserved requests: %d\n", cnt);

        delete[] chi;
        for (int i = 0; i < numTrips; i++) {
            delete[] epsilon[i];
        }
        delete[] epsilon;
    }
};

RTVGraph* RTVGraph::TIdxComparable::rtvGraph;

#endif