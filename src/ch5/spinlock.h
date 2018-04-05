//
// Created by manolee on 3/19/18.
//

#ifndef SANDMP_SPINLOCK_H
#define SANDMP_SPINLOCK_H


#include <atomic>

class spinlock_mutex {

    //most basic atomic type.
    //does not support copying / assignment / nothing BESIDES clear() and test-and-set
    //GUARANTEED TO BE LOCK-FREE
    std::atomic_flag flag;
public:
    spinlock_mutex():
            flag(ATOMIC_FLAG_INIT) //ATOMIC_FLAG MUST ALWAYS BE INITIALIZED LIKE THIS
    {}

    void lock()
    {
        //test-and-set:
        //write 1 (set) to a memory location and return its old value as a single atomic
        //why loop? because we want to succeed in writing 1 when the value used to be 0
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }


};


#endif //SANDMP_SPINLOCK_H
