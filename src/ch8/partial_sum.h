//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_PARTIAL_SUM_H
#define SANDMP_PARTIAL_SUM_H

#include <future>
#include <numeric>
#include <algorithm>
#include "join_threads.h"
#include "barrier.h"

template <typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_chunk
    {
        void operator()(Iterator begin,
                        Iterator last,
                        std::future<value_type>* previous_end_value,
                        std::promise<value_type>* end_value)
        {
            try
            {
                Iterator end = last;
                ++end;
                std::partial_sum(begin,end,begin);

                //am i the first chunk, or not?
                if(previous_end_value)
                {
                    //i am not the first one - need to wait for previous one to finish
                    //(which also waited for previous one, and so on -> almost serializing)
                    value_type& addend = previous_end_value->get();
                    //update the last element first to notify whoever is waiting for it
                    *last+=addend;
                    if(end_value)
                    {
                        end_value->set_value(*last);
                    }
                    //do the work for the rest of the elements -> i.e., [first, last)
                    std::for_each(begin,last,[addend](value_type& item)
                                 {
                                    item += addend;
                                 });
                }
                else if(end_value)
                {
                    //i am the last chunk (and I also aren't the only one generated)
                    end_value->set_value(*last);
                }
            }
            catch(...)
            {
                if(end_value)
                {
                    end_value->set_exception(std::current_exception());
                }
                else
                {
                    throw;
                }
            }
        }
    };

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

    typedef typename Iterator::value_type value_type;

    std::vector<std::thread> threads(num_threads - 1);
    //store the value of the last element in each chunk.
    std::vector<std::promise<value_type>> end_values(num_threads - 1);
    //retrieve the end value of the previous chunk
    std::vector<std::future<value_type>> previous_end_values;
    previous_end_values.reserve(num_threads - 1); //reserve memory w/o changing size. avoid reallocation
    join_threads joiner(threads);

    Iterator block_start = first;
    for(uint64_t i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_last = block_start;

        //point to last elem of (still) current block, NOT OVER IT
        std::advance(block_last,block_size - 1);
        threads[i] = std::thread(process_chunk(),
                                 block_start,
                                 block_last,
                                 (i!=0) ? &previous_end_values[i-1] : 0,
                                 &end_values[i]);
        block_start = block_last;
        ++block_start;
        previous_end_values.push_back(end_values[i].get_future());
    }
    Iterator final_element = block_start;
    std::advance(final_element,std::distance(block_start,last) - 1);
    process_chunk()(block_start,
                    final_element,
                    (num_threads > 1) ? &previous_end_values.back() : 0,
                    0);
}

#endif //SANDMP_PARTIAL_SUM_H
