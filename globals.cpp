#ifndef _GLOBALS_CPP_
#define _GLOBALS_CPP_

using namespace std;

int now_time;
int total_reqs, served_reqs;
long long total_dist, unserved_dist, raw_dist;
int total_wait_time, total_delay_time;

double travel_time;
int travel_cnt;
double travel_max;

int max_vehicle, max_capacity;

const int time_step = 30;
const int max_node = 264346;
const int max_wait_sec = 360;
const int max_delay_sec = max_wait_sec;
const int velocity = 84; // dm/s

const int penalty = max_delay_sec;
const int max_v_per_req = 30;

const double minimal = 1e-4;

#endif