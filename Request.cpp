#ifndef _REQUEST_CPP_
#define _REQUEST_CPP_ 

class Request {
public:
    int start, end;
    int reqTime, expectedOffTime;
    int scheduledOnTime, scheduledOffTime;
    bool onBoard;

    Request() {}

    Request(int start, int end, int reqTime) {
        this->start = start;
        this->end = end;
        this->reqTime = reqTime;
        this->onBoard = false;
        this->scheduledOnTime = -1;
    }
};

#endif