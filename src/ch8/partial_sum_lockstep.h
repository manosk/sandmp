//
// Created by manolee on 4/18/18.
//

#ifndef SANDMP_PARTIAL_SUM_LOCKSTEP_H
#define SANDMP_PARTIAL_SUM_LOCKSTEP_H

#include <functional>
#include <thread>
#include <sstream>
#include "join_threads.h"
#include "barrier.h"

/* 'The GPU way': Lockstep and barriers */
//(not exception-safe)
template <typename Iterator>
void parallel_partial_sum_lockstep(Iterator first, Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_element
    {
        void operator()(Iterator first,
                        //Iterator last,
                        std::vector<value_type>& buffer,
                        unsigned i,
                        barrier& b)
        {
            value_type& ith_element = *(first+i);
            bool update_source = false;

            std::stringstream str;
            str << "i = " << i;
            /**
             * 1st round: everybody reads the one right before.
             * 2nd round: survivors read the elem two positions away (which at this point is like reading two elems)
             * 3rd round: survivors read the elem four positions away (which at this point is like reading four elems)
             * ...
             */
            for(unsigned step = 0, stride = 1; stride <= i; ++step,stride*=2)
            {   //'step % 2' used to write to input or to tmp buffer

                str << " and stride = " << stride;


                //[specify which is the 'in' array (and position in it)
                value_type const& source = step % 2 ? buffer[i] : ith_element;
                //[specify which is the 'out' array (and position in it)
                value_type& dest = step % 2 ? ith_element : buffer[i];
                //get the value strides elements prior
                value_type const& addend = step % 2 ? buffer[i - stride] : *(first+i-stride);

                //sum addend and in[i], and store them to out[i]
                dest = source + addend;
                update_source = !(step%2);

                //wait for all to write.
                b.wait();
            }
            str << std::endl;
            std::cout << str.str();
            str.str("");
            if(update_source)
            {
                //update elem in original range if final result was stored in buffer
                ith_element = buffer[i];
            }
            //remove yourself from the barrier subscriber (e.g., if you are in position 1 and participated only once)
            b.done_waiting();
        }
    };

    uint64_t const length = std::distance(first,last);

    if(length <= 1)
    {
        return;
    }

    //alternate between writing to original storage and this helper buffer
    //(resembles process for merge sort)
    std::vector<value_type> buffer(length);
    barrier b(length);

    std::vector<std::thread> threads(length - 1);//Thread# dependent on #elems in list!!!
    join_threads joiner(threads);
    //Iterator block_start = first;
    for(uint64_t i = 0; i < (length - 1); ++i)
    {

        threads[i]=std::thread(process_element(),
                               first,
                               //last,
                               std::ref(buffer),
                               i,
                               std::ref(b));
    }
    process_element()(first,
                      //last,
                      buffer,
                      length - 1,
                      b);
}

#endif //SANDMP_PARTIAL_SUM_LOCKSTEP_H
