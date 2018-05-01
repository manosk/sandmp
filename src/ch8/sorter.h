//
// Created by manolee on 4/9/18.
//

#ifndef SANDMP_SORTER_H
#define SANDMP_SORTER_H

#include <list>
#include <future>
#include <boost/shared_ptr.hpp>
#include "ch6/queue.h"

/**
 * Thread-pool-like approach for quicksort
 *
 * Potential issue: Contention on stack of tasks (the 'chunks' data structure)
 */
template <typename T>
struct sorter
{
    struct chunk_to_sort
    {
        std::list<T> data;
        std::promise<std::list<T>> promise;

        chunk_to_sort() = default;

        chunk_to_sort(const chunk_to_sort&) = delete;

        chunk_to_sort(chunk_to_sort&& source) :
                data(std::move(source.data)),
                promise(std::move(source.promise))
        {}
    };

    concurrentLockWait::queue<chunk_to_sort> chunks;
    std::vector<std::thread> threads;
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;

    sorter()    :
            max_thread_count(std::thread::hardware_concurrency() - 1),
            end_of_data(false)
    {}

    ~sorter()
    {
        end_of_data = true;
        for(unsigned i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }
    }

    void try_sort_chunk()
    {
        std::shared_ptr<chunk_to_sort> chunk = chunks.try_pop(); //.pop();
        if(chunk)
        {
            sort_chunk(chunk);
        }
    }

    std::list<T> do_sort(std::list<T>& chunk_data)
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(),chunk_data,chunk_data.begin());
        //Use first element of input as the pivot; splice it in result
        //NOTE on splice: Transfers elements from one list to another.
        //No elements are copied or moved, only the internal pointers of the list nodes are re-pointed
        //('input' now no longer contains pivot)
        T const& pivot = *result.begin();

        //divide_point: iterator marking the first element that is NOT LESS than the pivot value
        typename std::list<T>::iterator divide_point =
                std::partition(chunk_data.begin(),chunk_data.end(),
                [&](T const& val){return val < pivot;});

        //create list containing the 'bottom half' elements. chunk_data now only contains the rest
        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(),
                                    chunk_data,
                                    chunk_data.begin(),
                                    divide_point);

        std::future<std::list<T>> new_lower = new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));
        //Span a new thread while we have PROCESSORS TO SPARE
        if(threads.size() < max_thread_count)
        {
            threads.push_back(std::thread(&sorter<T>::sort_thread,this));
        }


        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(),new_higher);
        //Can't splice new_lower if it is not done yet => try to sort some chunks in the meantime
        while(new_lower.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready)
        {
            //process chunks yourself (i.e., in the current thread) if you are waiting
            try_sort_chunk();
        }

        //Stitch stuff together and exit
        result.splice(result.begin(),new_lower.get());
        return result;
    }

    void sort_chunk(std::shared_ptr<chunk_to_sort> const& chunk)
    {
        chunk->promise.set_value(do_sort(chunk->data));
    }

    void sort_thread()
    {
        //Loop will terminate once sorter gets destructed at the end of parallel_quick_sort
        //All threads that got launched will be trying to pop work chunk from the stack till the end of exec.
        while(!end_of_data)
        {
            try_sort_chunk();
            std::this_thread::yield();
        }
    }
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}

#endif //SANDMP_SORTER_H
