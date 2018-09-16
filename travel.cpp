#ifndef _TRAVEL_CPP_
#define _TRAVEL_CPP_

#include <cstdio>
#include <cassert>
#include <ctime>
#include <map>
#include <set>
#include <algorithm>
#include "globals.cpp"
#include "GPtree.cpp"
#include "Vehicle.cpp"
#include "Request.cpp"
using namespace std;

int ansCost;
int ansTravelled;
vector<pair<int, int> > ansPath;
vector<Request> ansSchedule;

void dfs(Vehicle& vehicle, Request *reqs[], int numReqs,
    set<int>& target, map<int, vector<int> >& src_dst,
    vector<pair<int, int> >& path, vector<Request>& schedule,
    map<pair<int, int>, int> *dist,
    int travelled, int nowCost, int nowTime, bool decided) {

    if (target.size() == 0) {
        if (nowCost < ansCost) {
            ansCost = nowCost;
            ansTravelled = travelled;
            if (decided) {
                ansPath = path;
                ansSchedule = schedule;
            }
        }
        return;
    }

    int prevLoc = vehicle.get_location();
    vector<int> tmpTarget(target.begin(), target.end());
    vector<int>::iterator iter = tmpTarget.begin();
    vector<int> getOns, getOffsReq, getOffsPsngr, inserted;

    // try to arrive at a target
    for (int idx = 0; idx < tmpTarget.size(); idx++) {
        int node = tmpTarget[idx];

        int interDist = get_dist(prevLoc, node, dist);
        int newTime = nowTime + interDist / velocity;

        for (int i = 0; i < numReqs; i++) {
            // exceed max waiting time
            if (!reqs[i]->onBoard && newTime > reqs[i]->reqTime + max_wait_sec) {
                idx++;
                continue;
            }
        }

        getOns.clear();
        getOffsReq.clear();
        getOffsPsngr.clear();
        inserted.clear();

        bool visited = false;
        for (vector<pair<int, int> >::iterator it = path.begin();
            it != path.end(); it++) {
            
            if (it->second == node) {
                visited = true;
                break;
            }
        }
        // not visited and is a get-on node
        if (!visited && src_dst.find(node) != src_dst.end()) {
            vector<int>::iterator iterDst = src_dst[node].begin();
            while (iterDst != src_dst[node].end()) {
                if (target.insert(*iterDst).second) {
                    // record nodes newly inserted into target
                    inserted.push_back(*iterDst);
                }
                iterDst++;
            }
            for (int i = 0; i < numReqs; i++) {
                if (reqs[i]->start == node && !reqs[i]->onBoard) {
                    reqs[i]->onBoard = true;
                    if (decided) {
                        reqs[i]->scheduledOnTime = newTime;
                    }
                    // record who got on
                    getOns.push_back(i);
                }
            }
        }

        int newCost = nowCost;
        bool exceeded = false;

        for (int i = 0; i < numReqs; i++) {
            if (reqs[i]->onBoard) {
                // total delay time exceeded
                if (newTime - reqs[i]->expectedOffTime > max_delay_sec) {
                    exceeded = true;
                    break;
                }
                if (reqs[i]->end == node) {
                    reqs[i]->onBoard = false;
                    if (decided) {
                        reqs[i]->scheduledOffTime = newTime;
                        schedule.push_back(*reqs[i]);
                    }
                    newCost += newTime - reqs[i]->expectedOffTime;
                    // record who got off
                    getOffsReq.push_back(i);
                }
            }
        }
        // check if any "old" passenger arrived
        vehicle.check_passengers(newTime, node, exceeded, newCost,
            getOffsPsngr, schedule, decided);
        if (newCost >= ansCost) {
            exceeded = true;
        }

        if (!exceeded) {
            path.push_back(make_pair(newTime, node));
            target.erase(node);
            vehicle.set_location(node);
            
            dfs(vehicle, reqs, numReqs, target, src_dst, path, schedule, dist, travelled + interDist, newCost, newTime, decided);

            vehicle.set_location(prevLoc);
            target.insert(node);
            path.pop_back();
        }

        // restore attribute "onBoard" of recorded reqs
        vector<int>::iterator iterRec;
        for (iterRec = getOns.begin(); iterRec != getOns.end(); iterRec++) {
            // printf("%d ", *iterRec);
            reqs[*iterRec]->onBoard = false;
        }
        // printf("\n## a\n");
        vehicle.reverse_passengers(getOffsPsngr, schedule, decided);
        for (iterRec = getOffsReq.begin(); iterRec != getOffsReq.end(); iterRec++) {
            // printf("%d ", *iterRec);
            reqs[*iterRec]->onBoard = true;
        }
        size_t offCnt = getOffsReq.size();
        while (offCnt > 0) {
            schedule.pop_back();
            offCnt--;
        }
        for (iterRec = inserted.begin(); iterRec != inserted.end(); iterRec++) {
            target.erase(*iterRec);
        }
        // printf("\n## b\n");
        idx++;
    }
}

int travel(Vehicle& vehicle, Request *reqs[], int numReqs,
map<pair<int, int>, int> *dist, bool decided) {

    clock_t beginClock = clock();
    travel_cnt++;

    set<int> target; // nodes going to arrive at
    map<int, vector<int> > src_dst; // unhandled requests
    // insert new requests: s->t into src_dst
    for (int i = 0; i < numReqs; i++) {
        Request *req = reqs[i];
        if (src_dst.find(req->start) == src_dst.end()) {
            src_dst[req->start] = vector<int>();
        }
        src_dst[req->start].push_back(req->end);
        target.insert(req->start);
    }
    // insert targets of old passengers
    vehicle.insert_targets(target);

    ansCost = max_delay_sec * numReqs;
    ansTravelled = -1;

    vector<pair<int, int> > path;
    path.reserve(numReqs * 2 + 1);
    // path.push_back(vehicle.location);
    vector<Request> schedule;

    dfs(vehicle, reqs, numReqs, target, src_dst, path, schedule, dist, 0, 0,
        now_time + vehicle.get_time_to_next_node(), decided);
    
    if (ansTravelled >= 0) {
        if (decided) {
            int tmp = numReqs + vehicle.get_num_passengers();
            int schcnt = ansSchedule.size();
            vehicle.set_path(ansPath);
            vehicle.set_passengers(ansSchedule);
        }
        clock_t endClock = clock();
        double this_time = (double(endClock - beginClock)) / CLOCKS_PER_SEC;
        travel_max = max(travel_max, this_time);
        travel_time += this_time;
        // for (int i = 0; i < ansPath.size(); i++) {
            // printf("%d, ", ansPath[i]);
        // }
        // printf("\n");
        // printf("%d\n", ansTravelled);
        return ansCost;
    } else {
        clock_t endClock = clock();
        double this_time = (double(endClock - beginClock)) / CLOCKS_PER_SEC;
        travel_max = max(travel_max, this_time);
        travel_time += this_time;
        return -1;
    }
}

#endif