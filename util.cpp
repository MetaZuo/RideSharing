#ifndef _UTIL_CPP_
#define _UTIL_CPP_

#include <cstdio>
#include <map>
#include "GPtree.cpp"
using namespace std;

FILE* get_requests_file(const char* file) {
    FILE *in = NULL;
    in = fopen(file, "r");
    return in;
}

void read_vehicles(const char* file, vector<Vehicle>& vehicles) {
    vehicles.clear();
    FILE *in = NULL;
    in = fopen(file, "r");
    int loc;
    int num = 0;
    max_vehicle = 0;
    while (num != EOF) {
        num = fscanf(in, "%d\n", &loc);
        if (num != EOF) {
            vehicles.push_back(Vehicle(loc));
            max_vehicle++;
        }
    }
    printf("max vehicle = %d\n", max_vehicle);
    fclose(in);
}

bool read_requests(FILE*& in, vector<Request>& requests, int toTime,
    map<pair<int, int>, int> *dist) {
    
    int num = 0;
    int start, end, reqTime;
    while (num != EOF) {
        num = fscanf(in, "%d,%d,%d\n", &reqTime, &start, &end);
        if (num != EOF) {
            Request r(start, end, reqTime);
            r.shortestDist = get_dist(start, end, dist);
            r.expectedOffTime = reqTime + r.shortestDist / velocity;
            requests.push_back(r);
            raw_dist += r.shortestDist;
            total_reqs++;
            if (reqTime > toTime) {
                return true;
            }
        }
    }
    return false;
}

void handle_unserved(vector<Request>& unserved, vector<Request>& requests,
    int nowTime) {
    
    vector<Request>::iterator iter = unserved.begin();
    for (; iter != unserved.end(); iter++) {
        if (nowTime - iter->reqTime <= max_wait_sec) {
            requests.push_back(*iter);
        } else {
            unserved_dist += iter->shortestDist;
        }
    }
}

void update_vehicles(vector<Vehicle>& vehicles, vector<Request>& requests,
    int nowTime, map<pair<int, int>, int> *dist) {
    
    int idx = 0;
    vector<Vehicle>::iterator it = vehicles.begin();
    for (; it != vehicles.end(); it++) {
        // printf("V #%d: ", idx++);
        it->update(nowTime, requests, dist);
    }
}

void finish_all(vector<Vehicle>& vehicles, vector<Request>& unserved,
    map<pair<int, int>, int> *dist) {
    
    int idx = 0;
    vector<Vehicle>::iterator it = vehicles.begin();
    for (; it != vehicles.end(); it++) {
        // printf("V #%d: ", idx++);
        it->finish_route(dist);
    }
    vector<Request>::iterator iter = unserved.begin();
    for (; iter != unserved.end(); iter++) {
        unserved_dist += iter->shortestDist;
    }
}

void print_stats() {
    printf("\nService rate: %d / %d = %f\n",
        served_reqs, total_reqs, (double(served_reqs)) / total_reqs);
    /*
    for (set<int>::iterator it = servedUids.begin(); it != servedUids.end(); it++) {
        printf("%d, ", *it);
    }
    printf("\n");
     */
    printf("Dratio = %f\n", double(total_dist + unserved_dist) / raw_dist);
    printf("Eratio = %f\n", double(total_dist) / (raw_dist - unserved_dist));
    printf("Total waiting time = %d\n", total_wait_time);
}

#endif