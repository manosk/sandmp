//
// Created by manolee on 3/12/18.
//



#include "double_buffer.h"

void produce(double_buffer<int>* db);
void produce(double_buffer<int>* db)
{
    std::stringstream str;
    for (int i = 1; i <= 19; ++i) {
        str << "Produced " << i << std::endl;
        db->write(i);
        std::cout << str.str();
        str.str("");
    }
    //Flushing whatever is leftover
    db->flush();
}

void consume(double_buffer<int>* db);
void consume(double_buffer<int>* db)
{
    std::stringstream str;
    while(true) {
        int res = db->read();
        str << "[consumer] read " << res << " elements" << std::endl;
        std::cout << str.str();
        str.str("");
        if(res < CAPACITY) {
            str << "[consumer] No more elements; quitting" << std::endl;
            std::cout << str.str();
            str.str("");
            break;
        }

        //Simulating (very) slow readers
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3s);
    }
}

void run_double_buffer()
{
    double_buffer<int> db;
    std::thread t1(produce,&db);
    std::thread t2(consume,&db);
    t1.join();
    t2.join();
}