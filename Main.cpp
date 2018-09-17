#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "gurobi_c++.h"

#include "globals.cpp"
#include "Request.cpp"
#include "Vehicle.cpp"
#include "RV.cpp"
#include "RTV.cpp"
#include "GPtree.cpp"
#include "util.cpp"
using namespace std;


int main(int argc, char* argv[]) {

    char* reqFile = argv[1];
    char* vehFile = argv[2];
    char* outFile = argv[3];
    max_capacity = atoi(argv[4]);
    
    GRBEnv *env = new GRBEnv();
    env->set(GRB_IntParam_OutputFlag, 0);
    now_time = 0;
    total_reqs = served_reqs = 0;
    total_dist = unserved_dist = raw_dist = 0;
    total_wait_time = 0;

    map<pair<int, int>, int> *dist = new map<pair<int, int>, int>;
    initialize(true);
    printf("load_end\n\n");

    vector<Vehicle> vehicles;
    vehicles.reserve(max_vehicle);
    read_vehicles(vehFile, vehicles);

    vector<Request> requests;
    requests.reserve(100);

    bool hasMore = false;
    Request tail;

    vector<Request> unserved;

    FILE *in = get_requests_file(reqFile);

    while (true) {
        travel_time = 0;
        travel_max = 0;
        travel_cnt = 0;
        now_time += time_step;
        // printf("\nNow time = %d\n", now_time);

        clock_t tick = clock();

        requests.clear();
        if (hasMore) {
            requests.push_back(tail);
        }

        handle_unserved(unserved, requests, now_time);
        if (read_requests(in, requests, now_time, dist)) {
            tail = requests.back();
            requests.pop_back();
            hasMore = true;
        } else {
            hasMore = false;
        }
        update_vehicles(vehicles, requests, now_time, dist);

        clock_t t1 = clock();
        // printf("preprocessing time = %f\n", (double(t1 - tick)) / CLOCKS_PER_SEC);

        RVGraph *RV = new RVGraph(vehicles, requests, dist);

        clock_t t2 = clock();
        // printf("RV Building time = %f\n", (double(t2 - t1)) / CLOCKS_PER_SEC);

        RTVGraph *RTV = new RTVGraph(RV, vehicles, requests, dist);

        clock_t t3 = clock();
        // printf("RTV Building time = %f\n", (double(t3 - t2)) / CLOCKS_PER_SEC);

        RTV->solve(env, vehicles, requests, unserved, dist);

        clock_t t4 = clock();
        // printf("Solving time = %f\n", (double(t4 - t3)) / CLOCKS_PER_SEC);

        RTV->rebalance(env, vehicles, unserved, dist);

        clock_t t5 = clock();
        // printf("Rebalancing time = %f\n", (double(t5 - t4)) / CLOCKS_PER_SEC);

        delete RV;
        delete RTV;

        clock_t tock = clock();
        // printf("travel / total = %f / %f\n", travel_time, (double(tock - tick)) / CLOCKS_PER_SEC);

        if (!hasMore) {
            break;
        }
    }
    finish_all(vehicles, unserved, dist);

    print_stats(outFile);

    fclose(in);

    delete dist;
    delete env;

    return 0;
}