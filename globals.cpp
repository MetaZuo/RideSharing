#ifndef _GLOBALS_CPP_
#define _GLOBALS_CPP_

int now_time;

const int time_step = 60;
const int max_node = 264346;
const int max_vehicle = 400;
const int max_wait_sec = 300;
const int max_delay_sec = 2 * max_wait_sec;
const int max_capacity = 4;
const int velocity = 200; // dm/s

const int penalty = 100000;

const double minimal = 1e-4;

#endif