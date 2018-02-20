//
// Created by manolee on 2/20/18.
//
#include "basics.h"

void hello()
{
    std::cout << "Hello concurrent world" << std::endl;
}

void updateSomeVal(int &val)
{
    std::cout << "Thread updating val" << std::endl;
    val = 99;
}

void processDynObject(std::unique_ptr<int> obj)
{
    std::cout << "Dynamic 'object': " << *obj << std::endl;
}

void run_ch2_basics()
{

    //Plain way to create and launch thread
    {
        std::thread t(hello);

        //Wait for t to complete, and disassociate (formerly) running thread with object t
        //Alternative: Detach! => Thread will continue running even after the destruction of object t
        t.join();
    }

    //Using a callable object
    {
        background_task f;
        std::thread my_thread(f);
        my_thread.join();
    }

    //Using a callable anonymous object
    {
        //Note the use of uniform initialization syntax
        std::thread my_thread{background_task()};
        my_thread.join();
    }

    //Using a lambda
    {
        std::thread my_thread([]{
            std::cout << "Threading via lambda" << std::endl;
        });
        my_thread.join();
    }

    //Using a thread guard
    {
        std::thread t(hello);
        thread_guard g(t);
    }

    //Treating refs as... well... refs :)
    {
        int v = 0;
        //Actually, code won't even compile w/o std::ref -> type deduction crash (?)
        std::thread t1(updateSomeVal,std::ref(v));
        t1.join();
        std::cout << "v should have changed -> " << v << std::endl;
    }

    //Transfering ownership of a dynamic object into a thread
    {
        auto obj = std::make_unique<int>(114);
        std::thread t(processDynObject,std::move(obj));
        t.join();
        //what obj holds is now undefined
    }

    //Showcasing threads are moveable
    {
        std::thread t(hello);
        scoped_thread g(t);
    }

    //Showcasing threads are moveable, ptII: Placing them in container supporting move ops
    {
        std::vector<std::thread> threads;
        for(auto i = 0; i < 20; i++)
        {
            threads.emplace_back(hello);
        }
        for(auto& t : threads)  {
            t.join();
        }
    }
}