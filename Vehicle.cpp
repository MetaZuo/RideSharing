#ifndef _VEHICLE_CPP_
#define _VEHICLE_CPP_

#include <vector>
#include <set>
#include "globals.cpp"
using namespace std;

class Vehicle {
    int location, timeToNextNode;
    vector<Request> passengers;
    vector<pair<int, int> > scheduledPath;

public:
    Vehicle(){
        timeToNextNode = 0;
    }

    Vehicle(int location) {
        this->location = location;
        this->timeToNextNode = 0;
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
            printf("%d, ", passengers[i].unique);
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

    void update(int nowTime, vector<Request>& newRequests) {
        if (this->scheduledPath.empty()) {
            return;
        }
        vector<pair<int, int> >::iterator it = this->scheduledPath.begin();
        bool finished = true;
        while (it != this->scheduledPath.end()) {
            int schedTime = it->first;
            int node = it->second;
            if (schedTime >= nowTime) {
                this->timeToNextNode = schedTime - nowTime;
                this->location = node;
                finished = false;
                break;
            }
            it++;
        }
        if (finished) {
            this->timeToNextNode = 0;
            this->location = this->scheduledPath.back().second;
            this->scheduledPath.clear();
        }

        int nextArriveTime = nowTime + this->timeToNextNode;

        vector<Request> newPassengers;
        vector<Request>::iterator iterPsngr = this->passengers.begin();
        for (; iterPsngr != this->passengers.end(); iterPsngr++) {
            // hasn't got on board
            if (iterPsngr->scheduledOnTime > nextArriveTime) {
                newRequests.push_back(*iterPsngr);
                printf("%d waiting, ", iterPsngr->unique);
            } else if (iterPsngr->scheduledOffTime <= nextArriveTime) {
                // already got off
                // TODO add into results
                served_reqs++;
                printf("%d off, ", iterPsngr->unique);
                servedUids.insert(iterPsngr->unique);
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
        this->scheduledPath = path;
    }

    void finish_route() {
        vector<Request>::iterator iterPsngr = this->passengers.begin();
        for (; iterPsngr != this->passengers.end(); iterPsngr++) {
            served_reqs++;
            // printf("%d on, ", iterPsngr->unique);
            servedUids.insert(iterPsngr->unique);
        }
        // printf("\n");
    }
};

#endif