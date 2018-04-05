//
// Created by manolee on 3/1/18.
//

#ifndef SANDMP_LOCK_H
#define SANDMP_LOCK_H

#include <list>
#include <mutex>
#include <algorithm>
#include <iostream>

#include "threadsafe_stack.h"
#include "lock_pair.h"
#include "hierarchical_mutex.h"
#include "readerwriter_mutex.h"

/**
 * Material:
 * mutexes
 * lock_guard(mutex)
 * lock(mutex...)
 * adopt_lock
 * unique_lock(mutex)
 * defer_lock
 * once_flag, call_once
 * (boost) shared_mutex
 *
 * recursive_mutex -> not recommended (reason: invariants broken, and lock still succeeds)
 */

void add_to_list(int new_value);

bool list_contains(int value_to_find);

void run_ch3_locks();


#endif //SANDMP_LOCK_H
