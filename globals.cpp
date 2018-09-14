#ifndef _GLOBALS_CPP_
#define _GLOBALS_CPP_

#include <set>
using namespace std;

int now_time;
int total_reqs, served_reqs;

set<int> servedUids;

const int time_step = 120;
const int max_node = 264346;
const int max_vehicle = 1000;
const int max_wait_sec = 360;
const int max_delay_sec = 2 * max_wait_sec;
const int max_capacity = 4;
const int velocity = 84; // dm/s

const int penalty = 1000000;

const double minimal = 1e-4;

#endif