//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_FOREACH_H
#define SANDMP_FOREACH_H

#include <iterator>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include "join_threads.h"

template <typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
    uint64_t const length = std::distance(first,last);

    if(!length)
    {
        return;
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

    std::vector<std::future<void>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for(uint64_t i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end,block_size);
        std::packaged_task<void(void)> task(
                [=]()
                {
                    std::for_each(block_start,block_end,f);
                }
        );
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        block_start = block_end;
    }

    std::for_each(block_start,last,f);
    for(uint64_t i = 0; i < (num_threads - 1); ++i)
    {
        futures[i].get();
    }
}

template <typename Iterator, typename Func>
void parallel_for_each_async(Iterator first, Iterator last, Func f) {
    uint64_t const length = std::distance(first, last);

    if (!length) {
        return;
    }

    uint64_t const min_per_thread = 25;

    if(length < 2 * min_per_thread)
    {
        std::for_each(first,last,f);
    }
    else
    {
        Iterator const mid_point = first + length/2;
        std::future<void> first_half =
                std::async(&parallel_for_each_async<Iterator,Func>,
                           first,
                           mid_point,
                           f);
        parallel_for_each_async(mid_point,last,f);
        first_half.get();
    }
}


#endif //SANDMP_FOREACH_H
