//
// Created by manolee on 2/20/18.
// 'Basic thread management'
//

#ifndef SANDMP_BASICS_H
#define SANDMP_BASICS_H

#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>

/**
 *  Material:
 *  join()
 *  detach()
 *  joinable()
 *  thread_guard() ---> RAII
 *  scoped_thread()
 *  std::ref for (true) pass-by-ref
 *  thread movement (via std::move and emplace_back)
 */


void hello();

class background_task
{
public:
    //A "const function", denoted with the keyword const after a function declaration,
    //makes it a compiler error for this class function to change a member variable of the class.
    void operator()() const
    {
        std::cout << "Callable class" << std::endl;
    }
};

//Use RAII to wait for thread to complete.
//Protects again unexpected call to destructor of std::thread that corresponds to joinable thread
//-> std::thread destruction would lead to program termination (std::terminate)
//Resembles pattern from Item 37 of 'Effective Modern C++'
//Note that you only have to make this decision before the std::thread object is destroyed;
//the thread itself may well have finished long before you join with it or detach it.
//I you detach it, then the thread may continue running long after the std::thread object is destroyed.
class thread_guard
{
    std::thread &t;
public:
    //No implicit conversions / copy initialization allowed
    explicit thread_guard(std::thread &t_):  t(t_)
    {}
    ~thread_guard()
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    //being able to copy the guard could lead to a thread_guard outliving the scope of the thread it is joining
    thread_guard(thread_guard const&)=delete;
    thread_guard& operator=(thread_guard const&)=delete;
};

class scoped_thread
{
    std::thread t;
public:
    //No implicit conversions / copy initialization allowed
    explicit scoped_thread(std::thread &t_):  t(std::move(t_))
    {
        std::cout << "[ScopedThread] 'Wrapping' constructor" << std::endl;
        if(!t.joinable())
        {
            throw std::logic_error("No Thread");
        }
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&)=delete;
    scoped_thread& operator=(scoped_thread const&)=delete;
};

//(Used to implement fold in parallel)
template <typename Iterator, typename T>
struct accumulate_block
{
    void operator() (Iterator first, Iterator last, T& result)
    {
        result = std::accumulate(first,last,result);
    }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init);

//Threads can have access to local variables (e.g., via ref members in a callable class),
//BUT THEIR ARGUMENTS ARE COPIED BY DEFAULT!! Need ref instead of a copy? ---> std::ref
void updateSomeVal(int &val);

void processDynObject(std::unique_ptr<int> obj);

void run_parallel_accumulator();

void run_ch2_basics();

#endif //SANDMP_BASICS_H
