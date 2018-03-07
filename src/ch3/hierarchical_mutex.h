//
// Created by manolee on 3/7/18.
//

#ifndef SANDMP_HIERARCHICAL_MUTEX_H
#define SANDMP_HIERARCHICAL_MUTEX_H

#include <mutex>
#include <climits>

//concept: users/methods always have to lock in order of decreasing hierarchy
//caveat: cannot really lock twice with same hierarchy
class hierarchical_mutex
{
    std::mutex internal_mutex;
    uint64_t const hierarchy_value;
    uint64_t previous_hierarchy_value;
    static thread_local uint64_t this_thread_hierarchy_value;

    void check_for_hierarchy_violation();

    void update_hierarchy_value();

public:
    explicit hierarchical_mutex(uint64_t value);

    void lock();

    void unlock();

    bool try_lock();
};


#endif //SANDMP_HIERARCHICAL_MUTEX_H
