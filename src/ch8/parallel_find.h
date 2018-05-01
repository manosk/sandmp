//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_PARALLEL_FIND_H
#define SANDMP_PARALLEL_FIND_H

#include <future>
#include <vector>
#include "join_threads.h"

//Doesn't guarantee that it will return the first match!!!
template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match)
{
    struct find_element
    {
        void operator()(Iterator begin,
                        Iterator end,
                        MatchType match,
                        std::promise<Iterator>* result,
                        std::atomic<bool>* done_flag)
        {
            try {
                //These condvar checks are bound to be quite expensive...
                for(;(begin != end) && !done_flag->load(); ++begin)
                {
                    if(*begin == match)
                    {
                        result->set_value(begin);
                        done_flag->store(true);
                        return;
                    }
                }
            }
            catch(...)
            {
                try {
                    result->set_exception(std::current_exception());
                    done_flag->store(true);
                }
                catch(...) //trying to set an already set promise throws an exception
                {}
            }
        }
    };

    uint64_t const length = std::distance(first,last);

    if(!length)
    {
        return last;
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

    //Only a single worker thread will be fast enough to set the promise!
    std::promise<Iterator> result;
    std::atomic<bool> done_flag(false);
    std::vector<std::thread> threads(num_threads - 1);
    {
        //place this code in a block so that we guarantee that all threads join before checking the result
        //(after all, we may not have any match => return last)
        join_threads joiner(threads);

        Iterator block_start = first;
        for(uint64_t i = 0; i < (num_threads - 1); ++i)
        {
            Iterator block_end = block_start;
            std::advance(block_end,block_size);
            threads[i] = std::thread(find_element(),
                                     block_start,
                                     block_end,
                                     match,
                                     &result,
                                     &done_flag);
            block_start = block_end;
        }
        find_element()(block_start,last,match,&result,&done_flag);
    }
    if(!done_flag.load())
    {
        //no match
        return last;
    }

    //return result (or stored exception)
    return result.get_future().get();
}

//Doesn't guarantee that it will return the first match!!!
template<typename Iterator, typename MatchType>
Iterator parallel_find_impl(Iterator first, Iterator last, MatchType match, std::atomic<bool>& done)
{
    try
    {
        uint64_t const length = std::distance(first,last);
        uint64_t const min_per_thread = 25;
        if(length < 2 * min_per_thread)
        {
            for(;(first != last) && !done.load(); ++first)
            {
                if(*first == match)
                {
                    done = true;
                    return first;
                }
            }
            return last;
        }
        else
        {
            Iterator const mid_point = first + (length/2);
            std::future<Iterator> async_result =
                    std::async(&parallel_find_impl<Iterator,MatchType>,
                               mid_point,
                               last,
                               match,
                               std::ref(done));
            Iterator const direct_result =
                    parallel_find_impl(first,mid_point,match,done);
            return (direct_result == mid_point) ? async_result.get() //i.e, direct call failed to find a match
                                                : direct_result;
        }
    }
    catch(...)
    {
        //This catch serves as an early stopping implementation
        //-> have all threads terminate quickly if an exception is thrown
        done = true;
        throw;
    }
}

template<typename Iterator, typename MatchType>
Iterator parallel_find_async(Iterator first, Iterator last, MatchType match)
{
    std::atomic<bool> done(false);
    return parallel_find_impl(first,last,match,done);
}
#endif //SANDMP_PARALLEL_FIND_H
