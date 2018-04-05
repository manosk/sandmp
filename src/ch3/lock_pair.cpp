//
// Created by manolee on 3/7/18.
//

#include "lock_pair.h"

void swap(some_object& lhs, some_object& rhs)   {
    int tmp = lhs.value;
    lhs.value = rhs.value;
    rhs.value = tmp;
}

void swapA(ObjWrapper& lhs, ObjWrapper& rhs)
{
    if(&lhs == &rhs)
    {
    return;
    }
    //Lock two or more mutexes at the same time
    std::lock(lhs.m,rhs.m); //all-or-nothing
    //std::adopt_lock indicates to the lock_guard that the 'global' lock is already taken;
    //'just adopt ownership of existing lock'
    std::lock_guard<std::mutex> lock_a(lhs.m,std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(rhs.m,std::adopt_lock);
    swap(lhs.wrappedObj,rhs.wrappedObj);
}

const some_object &ObjWrapper::getWrappedObj() const {
    return wrappedObj;
}

const some_object &ObjWrapperB::getWrappedObj() const {
    return wrappedObj;
}

void swapB(ObjWrapperB& lhs, ObjWrapperB& rhs)
{
    if(&lhs == &rhs)
    {
        return;
    }

    //Unique lock allows DEFERRING locking at construction time
    //PRO 1: unique_lock does not have to own the mutex, as opposed to the lock_guard)
    //PRO 2: unique_lock is MOVABLE -> Can pass it along from one op to another to TRANSFER MUTEX OWNERSHIP
    //PRO 3: unique_lock allows explicit unlock (can do 'early' unlocking, not waiting for out-of-scope destruction)
    //CON: This comes at extra storage and perf cost: An extra flag is stored (and checked) internally
    std::unique_lock<std::mutex> lock_a(lhs.m,std::defer_lock);
    std::unique_lock<std::mutex> lock_b(rhs.m,std::defer_lock);
    //Lock two or more mutexes at the same time
    std::lock(lhs.m,rhs.m); //all-or-nothing
    swap(lhs.wrappedObj,rhs.wrappedObj);
}