//
// Created by manolee on 3/7/18.
//

#ifndef SANDMP_LOCK_PAIR_H
#define SANDMP_LOCK_PAIR_H

#include <mutex>

class some_object   {
public:
    int value;
};

void swap(some_object& lhs, some_object& rhs);

class ObjWrapper
{
private:
    some_object wrappedObj;
    std::mutex m;
public:

    explicit ObjWrapper(some_object const& wo) : wrappedObj(wo) {};

    const some_object &getWrappedObj() const;

    friend void swapA(ObjWrapper& lhs, ObjWrapper& rhs);


};

class ObjWrapperB
{
private:
    std::mutex m;
    some_object wrappedObj;
public:

    explicit ObjWrapperB(some_object const& wo) : wrappedObj(wo) {};

    const some_object &getWrappedObj() const;

    friend void swapB(ObjWrapperB& lhs, ObjWrapperB& rhs);
};

#endif //SANDMP_LOCK_PAIR_H
