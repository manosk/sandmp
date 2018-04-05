//
// Created by manolee on 3/19/18.
//

#ifndef SANDMP_QUICKSORT_SEQ_H
#define SANDMP_QUICKSORT_SEQ_H

#include <iostream>
#include <list>
#include <algorithm>
#include <future>

//printing lists (for convenience)
std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list);


template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    //Use first element of input as the pivot; splice it in result
    //NOTE on splice: Transfers elements from one list to another.
    //No elements are copied or moved, only the internal pointers of the list nodes are re-pointed
    //('input' now no longer contains pivot)
    result.splice(result.begin(),input,input.begin());
    T const& pivot = *result.begin();

    //divide_point: iterator marking the first element that is NOT LESS than the pivot value
    auto divide_point = std::partition(input.begin(),input.end(),
    [&](T const& t){ return t < pivot; } );

    //create list containing the 'bottom half' elements. input now only contains the rest
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(),input,input.begin(),divide_point);

    //recursion
    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));

    //Stitch lists together. Remember that result already contained the pivot element
    result.splice(result.end(),new_higher);
    result.splice(result.begin(),new_lower);
    return result;
}

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(),input,input.begin());
    T const& pivot = *result.begin();

    //The fact that the partitioning part is still sequential is not ideal
    //most time is spent on partitioning after all
    auto divide_point = std::partition(input.begin(),input.end(),
                        [&](T const& t){ return t < pivot; } );

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(),input,input.begin(),divide_point);

    std::future< std::list<T> > new_lower(
            std::async(&parallel_quick_sort<T>,std::move(lower_part))
    );

    //use direct recursion
    auto new_higher(parallel_quick_sort(std::move(input)));

    result.splice(result.end(),new_higher);
    //Note the special handling of the future -> calling get()
    result.splice(result.begin(),new_lower.get());
    return result;
}

#endif //SANDMP_QUICKSORT_SEQ_H
