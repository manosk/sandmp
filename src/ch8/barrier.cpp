//
// Created by manolee on 4/18/18.
//

#include "barrier.h"

void barrier::wait()
{
    unsigned const gen = generation.load();
    if(!--spaces)
    {   //no more spaces

        //reset #spaces
        spaces = count.load();

        //signal the other threads that they can continue
        ++generation;
    }
    else
    {
        //spinlock, basically
        while(generation.load() == gen)
        {
            //don't hog CPU
            //give up the current timeslice and re-insert the thread into the scheduling queue
            std::this_thread::yield();
        }
    }
}

void barrier::done_waiting()
{
    --count;
    if(!--spaces)
    {   //The last thread of the batch: Reset before quitting
        spaces = count.load();
        ++generation;
    }
}