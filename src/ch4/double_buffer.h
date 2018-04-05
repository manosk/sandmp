//
// Created by manolee on 3/12/18.
//

#ifndef SANDMP_DOUBLE_BUFFER_H
#define SANDMP_DOUBLE_BUFFER_H

#include <array>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <algorithm>
#include <sstream>

#define CAPACITY 5

void run_double_buffer();

template <typename T> struct buffer {
    std::array<T,CAPACITY> data;
    int curr_contents = 0;
    std::mutex mut;
    bool flush = false; //use to simplify handling of final, half-full buffer sent by producer. > 1 ways to handle.

    bool full()  { return curr_contents == CAPACITY || flush; }
    bool empty() { return curr_contents == 0; }

};

template <typename T>
class double_buffer {
private:
    int buf_ctr = 0;
    buffer<T> buffer1;
    buffer<T> buffer2;

    std::condition_variable producer_done;
    std::condition_variable consumer_done;
public:
    void write(T value)
    {
        std::lock_guard<std::mutex> lk1(buffer1.mut);
        if(buf_ctr < CAPACITY)
        {
            buffer1.data[buf_ctr++] = value;
        }
        else
        {
            std::unique_lock<std::mutex> lk2(buffer2.mut);
            consumer_done.wait(lk2,[this]()->bool{return buffer2.empty();});

//            std::stringstream str;
//            str << "DOUBLE BUFF!" << std::endl;
//            std::cout << str.str();
//            str.str("");

            //Swap!
            buf_ctr = 0;
            buffer1.curr_contents = 0;
            buffer2.curr_contents = CAPACITY;
            std::swap((&buffer1)->data,(&buffer2)->data);
            //Don't forget to actually insert
            buffer1.data[buf_ctr++] = value;
            producer_done.notify_one();
        }
    }

    //Producer uses it to forward the last data in its (half-full) buffer
    void flush()
    {
        std::stringstream str;
        std::unique_lock<std::mutex> lk1(buffer1.mut);
        std::unique_lock<std::mutex> lk2(buffer2.mut);
        consumer_done.wait(lk2,[this]()->bool{return buffer2.empty();});

        buffer1.curr_contents = 0;
        buffer2.curr_contents = buf_ctr;

//        str << "The flush: " << buf_ctr << std::endl;
//        std::cout << str.str();
//        str.str("");

        std::swap((&buffer1)->data,(&buffer2)->data);
        buffer2.flush = true;
        buf_ctr = 0;
        producer_done.notify_one();
    }

    //For the sake of this example, just return the number of elems in the buffer
    int read()
    {
        std::unique_lock<std::mutex> lk2(buffer2.mut);
        producer_done.wait(lk2,[this]()->bool{return buffer2.full();});


        int ctr = 0;
        //too eager; always reads CAPACITY entries obviously :)
        //std::for_each(buffer2.data.begin(),buffer2.data.end(),[&ctr](T&){ctr++;return;});
        for(int i = 0; i < buffer2.curr_contents; i++)
        {
            ctr++;
        }
        buffer2.curr_contents = 0;

        consumer_done.notify_one();
        return ctr;
    }

    //Hacky way to ensure that the buffer contains the intended elements
    int readAndSum()
    {
        std::unique_lock<std::mutex> lk2(buffer2.mut);
        producer_done.wait(lk2,[this]()->bool{return buffer2.full();});

        T sum = 0;
        int ctr = 0;
        //too eager; always reads CAPACITY entries obviously :)
        //std::for_each(buffer2.data.begin(),buffer2.data.end(),[&ctr](T&){ctr++;return;});
        for(int i = 0; i < buffer2.curr_contents; i++)
        {
            sum += buffer2.data[i];
            ctr++;
        }
        buffer2.curr_contents = 0;

        std::stringstream str;
        str << "[consumer] checksum " << sum << std::endl;
        std::cout << str.str();
        str.str("");

        consumer_done.notify_one();
        return ctr;
    }
};


#endif //SANDMP_DOUBLE_BUFFER_H
