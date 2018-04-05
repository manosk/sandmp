//
// Created by manolee on 3/26/18.
//

#ifndef SANDMP_CONCURRENT_H
#define SANDMP_CONCURRENT_H
#include "ch6/queue.h"

void run_ch6_concurr();
void push(concurrentLockNoWait::queue<int>* q);
void pop(concurrentLockNoWait::queue<int>* q);
#endif //SANDMP_CONCURRENT_H
