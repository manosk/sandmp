//
// Created by manolee on 3/12/18.
//

#ifndef SANDMP_THREADSAFE_QUEUE_H
#define SANDMP_THREADSAFE_QUEUE_H

#include <memory> //for std::shared_ptr
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

template <typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut; //NEED IT TO BE MUTABLE TO BE ABLE TO 'MODIFY' IT IN empty()
    std::queue<T> data_queue;
    std::condition_variable has_elements;
public:
    threadsafe_queue()
    {}

    threadsafe_queue(const threadsafe_queue& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    threadsafe_queue& operator=(const threadsafe_queue&) = delete; //disallowing assignment for simplicity

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        has_elements.notify_one();
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
        {
            return false;
        }
        else
        {
            value = data_queue.front();
            data_queue.pop();
            return true;
        }
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
        {
            return std::make_shared<T>();
        }
        else
        {
            auto res = std::make_shared<T>(data_queue.front());
            data_queue.pop();
            return res;
        }
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        has_elements.wait(lk,[this]()->bool{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        has_elements.wait(lk,[this]()->bool{return !data_queue.empty();});
        auto res = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

void push(threadsafe_queue<int>* q);
void pop(threadsafe_queue<int>* q);
#endif //SANDMP_THREADSAFE_QUEUE_H
