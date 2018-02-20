//
// Created by manolee on 2/20/18.
//

#ifndef SANDMP_BASICS_H
#define SANDMP_BASICS_H

#include <iostream>
#include <thread>
#include <vector>

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
//Protects again unexpected call to destructor of joinable thread -> It'd lead to program termination (std::terminate)
//Resembles pattern from Item 37 of 'Effective Modern C++'
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

//Threads can have access to local variables (e.g., via ref members in a callable class),
//BUT THEIR ARGUMENTS ARE COPIED BY DEFAULT!! Need ref instead of a copy? ---> std::ref
void updateSomeVal(int &val);

void processDynObject(std::unique_ptr<int> obj);

void run_ch2_basics();

#endif //SANDMP_BASICS_H
