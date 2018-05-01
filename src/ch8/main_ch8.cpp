//
// Created by manolee on 4/9/18.
//

#include <iostream>
#include "ch8/partial_sum_lockstep.h"
#include "ch8/main_ch8.h"


void run_ch8()
{
    {
        std::list<int> input = {10, 4, 2, 1, 11, 22, 3, 15, 21};

        std::list<int> output = parallel_quick_sort(input);

        for (auto &i : output) {
            std::cout << " " << i;
        }
        std::cout << std::endl;
    }

    //run_parallel_accumulator()
    {
        std::vector<int> numbers(50);
        for (auto i = 0; i < 50; i++) {
            numbers[i] = i;
        }
        int result =
                exceptionsafe::parallel_accumulate<std::vector<int>::iterator,int>(numbers.begin(),numbers.end(),0);
        std::cout << "Result of exception-safe parallel fold is " << result << std::endl;
    }

    //run_parallel_accumulator_async()
    {
        std::vector<int> numbers(50);
        for (auto i = 0; i < 50; i++) {
            numbers[i] = i;
        }
        int result =
                exceptionsafe::parallel_accumulate_async<std::vector<int>::iterator,int>(numbers.begin(),numbers.end(),0);
        std::cout << "Result of exception-safe parallel fold (async) is " << result << std::endl;
    }

    {
        std::vector<int> numbers(50);
        for (auto i = 0; i < 50; i++) {
            numbers[i] = i;
        }

        parallel_partial_sum_lockstep<std::vector<int>::iterator>(numbers.begin(),numbers.end());

        std::cout << "Prefix sum a la GPU: " << std::endl;
        for (auto &i : numbers) {
            std::cout << " " << i;
        }
        std::cout << std::endl;
    }
}