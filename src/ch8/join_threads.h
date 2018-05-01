//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_JOIN_THREADS_H
#define SANDMP_JOIN_THREADS_H

#include <thread>
#include <vector>

//RAII taken to the 'vector' extreme
class join_threads
{
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>& threads_):
            threads(threads_)
    {}

    ~join_threads()
    {
        for(uint64_t i = 0; i < threads.size(); ++i)
        {
            if(threads[i].joinable())
            {
                threads[i].join();
            }
        }
    }
};


#endif //SANDMP_JOIN_THREADS_H
