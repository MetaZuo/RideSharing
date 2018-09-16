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
    map<int, vector<pair<int, int> > > req_cost_car;

    void add_reqs_edge(int r1, int r2) {
        req_inter[make_pair(r1, r2)] = true;
    }

    void add_edge_vehicle_req(int vehicle, int req, int cost) {
        car_req_cost[vehicle][req] = cost;
        req_cost_car[req].push_back(make_pair(cost, vehicle));
    }

    void prune() {
        map<int, vector<pair<int, int> > >::iterator it = req_cost_car.begin();
        for (; it != req_cost_car.end(); it++) {
            sort(it->second.begin(), it->second.end());
            if (it->second.size() <= max_v_per_req) {
                continue;
            }
            for (int idx = max_v_per_req; idx < it->second.size(); idx++) {
                int vId = it->second[idx].second;
                car_req_cost[vId].erase(it->first);
                if (car_req_cost[vId].empty()) {
                    car_req_cost.erase(vId);
                }
            }
        }
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
            if (vehicles[i].isAvailable()) {
                for (int j = 0; j < requests.size(); j++) {
                    reqs[0] = &requests[j];
                    int cost = travel(vehicles[i], reqs, 1, dist, false);
                    if (cost >= 0) {
                        add_edge_vehicle_req(i, j, cost);
                    }
                }
            }
        }
        prune();
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