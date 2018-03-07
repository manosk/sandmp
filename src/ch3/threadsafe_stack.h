//
// Created by manolee on 3/7/18.
//

#ifndef SANDMP_THREADSAFE_STACK_H
#define SANDMP_THREADSAFE_STACK_H

#include <list>
#include <mutex>

#include <algorithm>
#include <exception>
#include <memory>
#include <stack>

//(Double-ended queue used to implement std::stack)
/*
#include <deque>
template <typename T, typename Container = std::deque<T> >
class stack
{
    explicit stack(const Container&);
    explicit stack(Container&& = Container());
    template <class Alloc> explicit stack(const Alloc&);
    template <class Alloc> stack(const Container&, const Alloc&);
    template <class Alloc> stack(Container&&, const Alloc&);
    template <class Alloc> stack(stack&&, const Alloc&);

    bool empty() const;
    size_t size() const;
    T& top();
    T const& top() const;
    void push(T const&);
    void push(T&&);
    void pop();
    void swap(stack&&);
};
 */

struct empty_stack: std::exception
{
    const char* what() const throw()
    {
        return "empty stack";
    }
};

//Thread-safe stack
template <typename T>
class threadsafe_stack
{
private:
    //Common pattern: Place mutex and protected data in same class - encapsulate all uses of mutex
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() = default;
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    };
    //Assignment op is deleted
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }

    //(Thread safe:) Pop() return ref or ptr because their creation cannot throw an exception
    //If we were to use vanilla stack's return-value logic, we could have run into the following situation:
    // 1. pop value
    // 2a. copy it in order to return it to the function caller
    // 2b. EXCEPTION => value lost from stack too.
    //(Caveat) Now we have memory mgmt concerns, instead of just dealing with a value.
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        //Throwing an exception when empty is not the healthiest behavior in the world...
        //It'd be better to wait till somebody notifies you that there's now data to be popped (NEXT CHAPTER)
        if(data.empty()) throw empty_stack();
        std::shared_ptr<T> const res = std::make_shared<T>(data.top());
        data.pop();
        return res;
    }

    //(Caveat) Requires constructing an instance to pass in (can be expensive)
    //(Caveat) Stored type must be assignable (not all UDTs are)
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

#endif //SANDMP_THREADSAFE_STACK_H
