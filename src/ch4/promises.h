//
// Created by manolee on 3/23/18.
//

#ifndef SANDMP_PROMISES_H
#define SANDMP_PROMISES_H

#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>
#include <chrono>

//Example from http://en.cppreference.com/w/cpp/thread/promise
void accumulate(std::vector<int>::iterator first,
                std::vector<int>::iterator last,
                std::promise<int> accumulate_promise);

void do_work(std::promise<void> barrier);

void example_promises();

#endif //SANDMP_PROMISES_H
