#ifndef _VEHICLE_CPP_
#define _VEHICLE_CPP_

#include <vector>
#include <queue>
#include <set>
#include "globals.cpp"
#include "GPtree.cpp"
#include "Request.cpp"
using namespace std;

class Vehicle {
    int location, timeToNextNode;
    bool available;
    vector<Request> passengers;
    queue<pair<int, int> > scheduledPath;

public:
    Vehicle(){
        timeToNextNode = 0;
        available = true;
    }

    Vehicle(int location) {
        this->location = location;
        this->timeToNextNode = 0;
        available = true;
    }

    bool isAvailable() {
        return this->available;
    }

    int get_location() {
        return this->location;
    }

    void set_location(int location) {
        this->location = location;
    }

    int get_time_to_next_node() {
        return this->timeToNextNode;
    }

    int get_num_passengers() {
        return passengers.size();
    }

    void print_passengers() {
        for (int i = 0; i < passengers.size(); i++) {
            printf("%d: ", passengers[i].unique);
            if (passengers[i].onBoard) {
                printf("onboard, ");
            } else {
                printf("offboard, ");
            }
        }
        printf("\n");
    }

    void insert_targets(set<int>& target) {
        for (int i = 0; i < passengers.size(); i++) {
            target.insert(passengers[i].end);
        }
    }

    void check_passengers(int nowTime, int stop, bool& exceeded, int& sumCost,
        vector<int>& getOffPsngr, vector<Request>& schedule, bool decided) {
        for (int i = 0; i < passengers.size(); i++) {
            Request& req = passengers[i];
            if (req.onBoard) {
                if (nowTime - req.expectedOffTime > max_delay_sec) {
                    exceeded = true;
                    return;
                }
                if (req.end == stop) {
                    req.onBoard = false;
                    if (decided) {
                        req.scheduledOffTime = nowTime;
                        schedule.push_back(req);
                    }
                    sumCost += nowTime - req.expectedOffTime;
                    getOffPsngr.push_back(i);
                }
            }
        }
    }

    void reverse_passengers(vector<int>& getOffPsngr,
        vector<Request>& schedule, bool decided) {
        
        size_t offCnt = getOffPsngr.size();
        for (vector<int>::iterator it = getOffPsngr.begin(); it != getOffPsngr.end(); it++) {
            passengers[*it].onBoard = true;
        }
        if (decided) {
            while (offCnt > 0) {
                schedule.pop_back();
                offCnt--;
            }
        }
    }

    void set_passengers(vector<Request>& psngrs) {
        this->passengers = psngrs;
    }

    void head_for(int node, map<pair<int, int>, int> *dist) {
        vector<int> order;
        find_path(this->location, node, order);
        while (!this->scheduledPath.empty()) {
            this->scheduledPath.pop();
        }
        int tmpNode = this->location;
        int tmpTime = this->timeToNextNode;
        for (int i = 1; i < order.size(); i++) { // head is location itself
            int d = get_dist(tmpNode, order[i], dist);
            tmpTime += d / velocity;
            this->scheduledPath.push(make_pair(tmpTime, order[i]));
        }
    }

    void update(int nowTime, vector<Request>& newRequests,
        map<pair<int, int>, int> *dist) {
        
        if (this->scheduledPath.empty()) {
            return;
        }
        if (this->timeToNextNode < time_step) {
            while (!this->scheduledPath.empty()) {
                // TODO all these
                int schedTime = this->scheduledPath.front().first;
                int node = this->scheduledPath.front().second;

                if (schedTime < nowTime || schedTime - nowTime < time_step) {
                    if (!this->passengers.empty()) {
                        int onboardCnt = 0;
                        for (int i = 0; i < this->passengers.size(); i++) {
                            if (this->passengers[i].scheduledOnTime < schedTime) {
                                onboardCnt++;
                            }
                            if (this->passengers[i].scheduledOffTime < schedTime) {
                                onboardCnt--;
                            }
                        }
                        if (onboardCnt > 0) {
                            total_dist += get_dist(this->location, node, dist);
                        }
                    }
                    this->location = node;
                    this->scheduledPath.pop();
                }
                if (schedTime >= nowTime) {
                    this->timeToNextNode = schedTime - nowTime;
                    this->available = (this->timeToNextNode < time_step);
                    break;
                }
            }
        } else {
            this->timeToNextNode -= time_step;
            this->available = (this->timeToNextNode < time_step);
            if (this->available) {
                int schedTime = this->scheduledPath.front().first;
                int node = this->scheduledPath.front().second;
                if (!this->passengers.empty()) {
                    int onboardCnt = 0;
                    for (int i = 0; i < this->passengers.size(); i++) {
                        if (this->passengers[i].scheduledOnTime < schedTime) {
                            onboardCnt++;
                        }
                        if (this->passengers[i].scheduledOffTime < schedTime) {
                            onboardCnt--;
                        }
                    }
                    if (onboardCnt > 0) {
                        total_dist += get_dist(this->location, node, dist);
                    }
                }
                this->location = node;
                this->scheduledPath.pop();
            }
        }

        int baseTime = this->available ?
            nowTime + this->timeToNextNode
            : nowTime;
        vector<Request> newPassengers;
        vector<Request>::iterator iterPsngr = this->passengers.begin();
        for (; iterPsngr != this->passengers.end(); iterPsngr++) {
            // hasn't got on board
            if (iterPsngr->scheduledOnTime > baseTime) {
                newRequests.push_back(*iterPsngr);
                printf("%d waiting, ", iterPsngr->unique);
            } else if (iterPsngr->scheduledOffTime <= baseTime) {
                // already got off
                // TODO add into results
                served_reqs++;
                printf("%d off, ", iterPsngr->unique);
                total_wait_time += iterPsngr->scheduledOnTime - iterPsngr->reqTime;
            } else { // now on board
                iterPsngr->onBoard = true;
                printf("%d on, ", iterPsngr->unique);
                newPassengers.push_back(*iterPsngr);
            }
        }
        printf("\n");
        this->passengers = newPassengers;
    }

    void set_path(vector<pair<int, int> >& path) {
        while (!this->scheduledPath.empty()) {
            this->scheduledPath.pop();
        }
        vector<pair<int, int> >::iterator it = path.begin();
        for (; it != path.end(); it++) {
            this->scheduledPath.push(*it);
        }
    }

    void finish_route(map<pair<int, int>, int> *dist) {
        if (!this->passengers.empty()) {
            vector<Request>::iterator iterPsngr = this->passengers.begin();
            for (; iterPsngr != this->passengers.end(); iterPsngr++) {
                served_reqs++;
                // printf("%d on, ", iterPsngr->unique);
                total_wait_time += iterPsngr->scheduledOnTime - iterPsngr->reqTime;
            }
            // printf("\n");

            while (!this->scheduledPath.empty()) {
                int schedTime = this->scheduledPath.front().first;
                int node = this->scheduledPath.front().second;
                if (!this->passengers.empty()) {
                    int onboardCnt = 0;
                    for (int i = 0; i < this->passengers.size(); i++) {
                        if (this->passengers[i].scheduledOnTime < schedTime) {
                            onboardCnt++;
                        }
                        if (this->passengers[i].scheduledOffTime < schedTime) {
                            onboardCnt--;
                        }
                    }
                    if (onboardCnt > 0) {
                        total_dist += get_dist(this->location, node, dist);
                    }
                }
                this->location = node;
                this->scheduledPath.pop();
            }
        }
    }
};

#endif