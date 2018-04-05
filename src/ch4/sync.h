//
// Created by manolee on 3/12/18.
//

#ifndef SANDMP_SYNC_H
#define SANDMP_SYNC_H


#include <future>

#include "threadsafe_queue.h"
#include "double_buffer.h"
#include "quicksort.h"

int some_computation(int);

/**
 * Material: event synchronization
 * condition variables
 * double buffering (not in book; my draft)
 * futures via
 * --> async (true async and deferred)
 * --> std::packaged_task<T> (where T is function signature)
 * --> std::promise<T> (where T is some value)
 * exceptions 'for the future'
 * unique_future and shared_future
 * wait_for, wait_until. Why wait_until is preferrable to wait_for for condvars
 * std::chrono::steady_clock, system_clock, high_resolution_clock
 * std::chrono::duration<> and its typedefs: milliseconds, seconds, etc.
 * std::chrono::time_point<>
 * sleep_for, sleep_until, try_lock_for, try_lock_until
 */
void run_ch4_sync();

#endif //SANDMP_SYNC_H
