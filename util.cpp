#ifndef _UTIL_CPP_
#define _UTIL_CPP_

#include <cstdio>
#include <map>
#include "GPtree.cpp"
using namespace std;

int get_dist(int S, int T, map<pair<int, int>, int> *dist) {
    pair<int, int> st;
    if (S < T) {
        st = make_pair(S, T);
    } else {
        st = make_pair(T, S);
    }
    if (dist->find(st) != dist->end()) {
        return (*dist)[st];
    }
    int calculatedDist = search_cache(S - 1, T - 1);
    (*dist)[st] = calculatedDist;
    return calculatedDist;
}

FILE* get_requests_file(const char* file) {
    FILE *in = NULL;
    in = fopen(file, "r");
    char buf[150];
    fgets(buf, 150, in);
    return in;
}

void read_vehicles(const char* file, vector<Vehicle>& vehicles) {
    vehicles.clear();
    FILE *in = NULL;
    in = fopen(file, "r");
    int loc;
    for (int i = 0; i < max_vehicle; i++) {
        fscanf(in, "%d\n", &loc);
        vehicles.push_back(Vehicle(loc));
    }
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
            int shortest = get_dist(start, end, dist);
            r.expectedOffTime = reqTime + shortest / velocity;
            requests.push_back(r);
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
        }
    }
}

void update_vehicles(vector<Vehicle>& vehicles, vector<Request>& requests,
    int nowTime) {
    
    vector<Vehicle>::iterator it = vehicles.begin();
    for (; it != vehicles.end(); it++) {
        it->update(nowTime, requests);
    }
}

#endif