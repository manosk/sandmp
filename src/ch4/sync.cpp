//
// Created by manolee on 3/12/18.
//
#include "sync.h"


int some_computation(int arg)
{
    return arg + 42;
}


void run_ch4_sync()
{
//    {
//        threadsafe_queue<int> q;
//        std::thread t1(push, &q);
//        std::thread t2(pop, &q);
//        t1.join();
//        t2.join();
//    }

//    {
//        run_double_buffer();
//    }

//    {
//        std::future<int> the_answer = std::async(some_computation,911);
//
//        std::cout << "The answer is " << the_answer.get() << std::endl;
//
//        std::future<int> the_answer_true_async = std::async(std::launch::async,some_computation,911);
//
//        std::cout << "The (async) answer is " << the_answer_true_async.get() << std::endl;
//
//        std::future<int> the_answer_not_async = std::async(std::launch::deferred,some_computation,911);
//
//        std::cout << "The (not async) answer is " << the_answer_not_async.get() << std::endl;
//
//    }

//    {
//        std::list<int> list1 = {4, 2, 1, 5, 8, 9, 7};
//        std::list<int> sorted1 = sequential_quick_sort(list1);
//        std::list<int> sorted2 = parallel_quick_sort(list1);
//        std::cout << "sorted1: " << sorted1 << std::endl;
//        std::cout << "sorted2: " << sorted2 << std::endl;
//    }

    {
        bool expected = false;
        std::atomic<bool> b(true);

        while(!b.compare_exchange_weak(expected,true) && !expected)
        {
            std::cout << "Loop";
        }
        std::cout << b.load() << ", expected: " << expected << std::endl;
    }

    return;
}


