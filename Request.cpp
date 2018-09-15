#ifndef _REQUEST_CPP_
#define _REQUEST_CPP_ 

class Request {
public:
    int start, end;
    int shortestDist;
    int reqTime, expectedOffTime;
    int scheduledOnTime, scheduledOffTime;
    bool onBoard;
    int unique;

    static int nextUid;

    Request() {}

    Request(int start, int end, int reqTime) {
        this->start = start;
        this->end = end;
        this->reqTime = reqTime;
        this->onBoard = false;
        this->scheduledOnTime = -1;
        this->unique = nextUid++;
    }
};

int Request::nextUid = 0;

#endif