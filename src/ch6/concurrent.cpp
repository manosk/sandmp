//
// Created by manolee on 3/26/18.
//

#include <iostream>
#include <thread>
#include "concurrent.h"
#include "lookup_table.h"

void run_ch6_concurr()
{
    {
        concurrentLockNoWait::queue<int> q;
        std::cout << "Running?" << std::endl;
        q.push(8);
        q.push(17);
        std::cout << "Running" << std::endl;
        auto res = q.try_pop();
        std::cout << "About to pop" << std::endl;
        std::cout << "Popped " << *res << std::endl;
    }

    {
        concurrentLockNoWait::queue<int> q;
        std::thread th1(push,&q);
        std::thread th2(pop,&q);
        th1.join();
        th2.join();
    }

    {
        lookup_table<int,std::string> t;
        t.add_or_update_mapping(1,"hello");
        std::cout << "value for 1 is " << t.value_for(1) << std::endl;
        auto snapshot = t.get_map();
        std::cout << "[snapshot] value for 1 is " << snapshot[1] << std::endl;
    }
}

void push(concurrentLockNoWait::queue<int>* q)
{
    for (int i = 0; i < 10; ++i) {
        std::cout << "Pushing " << i << std::endl;
        q->push(i);
    }
}

void pop(concurrentLockNoWait::queue<int>* q)
{
    int i = 0;
    while (true) {
        if (std::shared_ptr<int> p = q->try_pop()) {
            std::cout << "Popping " << *p << std::endl;
            ++i;
        }
        if (i == 10) {
            break;
        }
    }
}