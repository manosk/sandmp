//
// Created by manolee on 4/17/18.
//

#ifndef SANDMP_ACCUMULATE_H
#define SANDMP_ACCUMULATE_H

#include <numeric>
#include <iterator>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include "join_threads.h"

namespace exceptionsafe {
    template<typename Iterator, typename T>
    struct accumulate_block
    {
        T operator()(Iterator first, Iterator last)
        {
            return std::accumulate(first,last,T());
        }
    };

    //uses packaged_task
    template<typename Iterator, typename T>
    T parallel_accumulate(Iterator first, Iterator last, T init)
    {
        //(unsigned long)
        uint64_t const length = std::distance(first,last);

        if(!length)
        {
            return init;
        }

        uint64_t const min_per_thread = 25;
        //Making sure division returns ceil()
        uint64_t const max_threads =
                (length + min_per_thread - 1) / min_per_thread;

        uint64_t const hardware_threads =
                std::thread::hardware_concurrency();

        uint64_t const num_threads =
                std::min(hardware_threads !=0 ? hardware_threads : 2,
                         max_threads);

        //Don't worry about the case in which number does not divide evenly;
        //the current thread will just be assigned the leftovers later on
        uint64_t const block_size = length / num_threads;

        std::vector<std::future<T>> futures(num_threads - 1);
        std::vector<std::thread> threads(num_threads - 1);
        //Avoid thread leakage and unexpected termination
        join_threads joiner(threads);

        Iterator block_start = first;
        for(uint64_t i = 0; i < (num_threads - 1); ++i)
        {
            Iterator block_end = block_start;
            std::advance(block_end,block_size);

//            auto bindArgs = std::bind(accumulate_block<Iterator,T>(),block_start,block_end);
//            std::packaged_task<T(Iterator,Iterator)> task(bindArgs);
//            futures[i] = task.get_future();
//            threads[i] = std::thread(std::move(task),block_start,block_end);

            using namespace std::placeholders;
            //(_1 and _2 are from std::placeholders, and represent future
            //arguments that will be passed
            auto bindArgs = std::bind(accumulate_block<Iterator,T>(),_1,_2);//block_start,block_end);
            std::packaged_task<T(Iterator,Iterator)> task(bindArgs);
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task),block_start,block_end);


            block_start = block_end;
        }
        T last_result = accumulate_block<Iterator,T>()(block_start,last);

        std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));

        T result = init;
        for(uint64_t i = 0; i < (num_threads - 1); ++i)
        {
            result += futures[i].get();
        }
        result += last_result;
        return result;
    }

    //uses async and handles exceptions completely internally
    //also uses recursive parallelism instead of horizontal, data-based parallelism
    template<typename Iterator, typename T>
    T parallel_accumulate_async(Iterator first, Iterator last, T init)
    {
        uint64_t const length = std::distance(first,last);
        uint64_t const max_chunk_size = 25;
        if(length <= max_chunk_size)
        {
            return std::accumulate(first,last,init);
        }
        else
        {
            Iterator mid_point = first;
            std::advance(mid_point,length/2);
            std::future<T> first_half_result = std::async(parallel_accumulate_async<Iterator,T>,
                                                          first,
                                                          mid_point,
                                                          init);
            T second_half_result = parallel_accumulate_async(mid_point,last,T());
            return first_half_result.get() + second_half_result;
        }
    }
}   /* namespace exceptionsafe */
#endif //SANDMP_ACCUMULATE_H
