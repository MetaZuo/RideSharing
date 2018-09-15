#ifndef _RV_CPP_
#define _RV_CPP_

#include <map>
#include <vector>
#include "Request.cpp"
#include "Vehicle.cpp"
#include "travel.cpp"
using namespace std;

class RVGraph {
    map<pair<int, int>, bool> req_inter;
    map<int, map<int, int> > car_req_cost;

    void add_reqs_edge(int r1, int r2) {
        req_inter[make_pair(r1, r2)] = true;
    }

    void add_edge_vehicle_req(int vehicle, int req, int cost) {
        car_req_cost[vehicle][req] = cost;
    }

public:
    RVGraph(vector<Vehicle>& vehicles, vector<Request>& requests,
    map<pair<int, int>, int> *dist) {
        Vehicle virtualCar = Vehicle();
        for (int i = 0; i < requests.size(); i++) {
            for (int j = i + 1; j < requests.size(); j++) {
                virtualCar.set_location(requests[i].start);
                Request *reqs[2] = {&requests[i], &requests[j]};
                int cost = travel(virtualCar, reqs, 2, dist, false);
                if (cost >= 0) {
                    add_reqs_edge(i, j);
                } else {
                    virtualCar.set_location(requests[j].start);
                    cost = travel(virtualCar, reqs, 2, dist, false);
                    if (cost >= 0) {
                        add_reqs_edge(i, j);
                    }
                }
            }
        }
        Request *reqs[1];
        for (int i = 0; i < vehicles.size(); i++) {
            for (int j = 0; j < requests.size(); j++) {
                reqs[0] = &requests[j];
                int cost = travel(vehicles[i], reqs, 1, dist, false);
                if (cost >= 0) {
                    add_edge_vehicle_req(i, j, cost);
                }
            }
        }
    }

    bool has_vehicle(int vehicle) {
        return (car_req_cost.find(vehicle) != car_req_cost.end());
    }

    int get_vehicle_num() {
        return int(car_req_cost.size());
    }

    bool has_reqs_edge(int r1, int r2) {
        return (req_inter.find(make_pair(r1, r2)) != req_inter.end());
    }

    void get_vehicle_edges(int vehicle, vector<pair<int, int> >& edges) {
        edges.clear();
        map<int, int>& req_costs = car_req_cost[vehicle];
        map<int, int>::iterator iter = req_costs.begin();
        while (iter != req_costs.end()) {
            edges.push_back(make_pair(iter->first, iter->second));
            iter++;
        }
    }
};

#endif