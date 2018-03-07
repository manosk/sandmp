//
// Created by manolee on 3/1/18.
//

#include "lock.h"

std::list<int> some_list;
std::mutex some_mutex;

//Locks entire list pretty much
void add_to_list(int new_value)
{
    //lock_guard is preferrable to explicit lock() and unlock(), because one might mistakenly not always couple them.
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

//Locks entire list pretty much
bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    return std::find(some_list.begin(),some_list.end(),value_to_find) != some_list.end();
}

void run_ch3_locks()    {

    {
        std::cout << "Threadsafe stack implementation" << std::endl;
        threadsafe_stack<int> si;
        si.push(5);
        si.push(9);
        int res1 = -1;

        if(!si.empty())
        {
            si.pop(res1);
            std::cout << "Just popped a value: " << res1 << std::endl;
        }

        if(!si.empty())
        {
            auto res2 = si.pop();
            std::cout << "Just popped a value: " << *res2 << std::endl;
        }
    }

    {
        std::cout << "**************************" << std::endl;
        std::cout << "Pairing two locks together" << std::endl;
        some_object obj1{10};
        some_object obj2{20};
        ObjWrapper ow1(obj1);
        ObjWrapper ow2(obj2);
        std::cout << "Before swap: " << ow1.getWrappedObj().value << "," << ow2.getWrappedObj().value << std::endl;
        swapA(ow1,ow2);
        std::cout << "After swap: " << ow1.getWrappedObj().value << "," << ow2.getWrappedObj().value << std::endl;

        std::cout << "**************************" << std::endl;
    }

    {
        std::cout << "**************************" << std::endl;
        std::cout << "Pairing two (unique_)locks together" << std::endl;
        some_object obj1{10};
        some_object obj2{20};
        ObjWrapperB ow1(obj1);
        ObjWrapperB ow2(obj2);
        std::cout << "Before swap: " << ow1.getWrappedObj().value << "," << ow2.getWrappedObj().value << std::endl;
        swapB(ow1,ow2);
        std::cout << "After swap: " << ow1.getWrappedObj().value << "," << ow2.getWrappedObj().value << std::endl;

        std::cout << "**************************" << std::endl;
    }

    {
        std::cout << "Attempting to obtain locks in the 'wrong' order" << std::endl;
        hierarchical_mutex m1(42);
        hierarchical_mutex m2(2000);

        std::lock_guard<hierarchical_mutex> g1(m1);
        std::lock_guard<hierarchical_mutex> g2(m2);
    }

}