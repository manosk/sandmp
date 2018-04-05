//
// Created by manolee on 3/12/18.
//

#include "threadsafe_queue.h"

void push(threadsafe_queue<int>* q)
{
    for (int i = 0; i < 10; ++i) {
        std::cout << "pushing " << i << std::endl;
        q->push(i);
    }
}

void pop(threadsafe_queue<int>* q)
{
    for (int i = 0; i < 10; ++i) {
        std::cout << "popping " << *q->wait_and_pop() << std::endl;
    }
}