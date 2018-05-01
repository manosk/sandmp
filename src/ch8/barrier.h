//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_BARRIER_H
#define SANDMP_BARRIER_H


#include <atomic>
#include <thread>

class barrier {
    std::atomic<unsigned> count; //atomic so that multiple threads can update its value
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;
public:
    explicit barrier(unsigned count_):
            count(count_), spaces(count_),generation(0)
    {}

    void wait();

    //allow a thread to 'unsubscribe' the barrier
    void done_waiting();
};


#endif //SANDMP_BARRIER_H
