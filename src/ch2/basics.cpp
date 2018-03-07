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

template <typename Iterator, typename T>
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

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for(uint64_t i = 0; i < (num_threads - 1); i++)
    {
        Iterator block_end = block_start;
        //The (safe) way to advance an iterator
        std::advance(block_end,block_size);
        //Note the use of std::ref to treat ref naturally (instead of just an lvalue I guess)
        threads[i] = std::thread(
          accumulate_block<Iterator,T>(),
          block_start,block_end,std::ref(results[i]));
        block_start = block_end;
    }
    //Handle the leftover of the division on current thread - no need to spawn another one
    accumulate_block<Iterator,T>()(
      block_start,last,results[num_threads-1]);

    //Wait for all threads to finish
    std::for_each(threads.begin(),threads.end(),std::mem_fn(&std::thread::join));

    //Accumulate final result
    return std::accumulate(results.begin(),results.end(),init);
}

void run_parallel_accumulator()
{
    std::vector<int> numbers(50);
    for (auto i = 0; i < 50; i++) {
        numbers[i] = i;
    }
    int result =
            parallel_accumulate<std::vector<int>::iterator,int>(numbers.begin(),numbers.end(),0);
    std::cout << "Result of parallel fold is " << result << std::endl;
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

    //Transferring ownership of a dynamic object into a thread
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

    {
        run_parallel_accumulator();
    }
}