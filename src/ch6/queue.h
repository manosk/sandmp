//
// Created by manolee on 3/26/18.
//

#ifndef SANDMP_QUEUE_H
#define SANDMP_QUEUE_H

#include <memory>
#include <mutex>
#include <condition_variable>

namespace naive {
    template<typename T>
    class queue {
    private:
        struct node {
            T data;
            std::unique_ptr<node> next;

            explicit node(T data_) :
                    data(std::move(data_)) {}
        };

        //ownership
        //also: automatically deleted when no longer pointed to by anyone
        std::unique_ptr<node> head;
        //duplicate ptr to 'some' node
        node *tail;

    public:
        queue() {
            tail = nullptr;
        }

        queue(const queue &other) = delete;

        queue &operator=(const queue &other)= delete;

        //ISSUE: head might be equal to tail when this method is called!!! -> Have to protect against it
        //-> code ends up being serialized if both pop and push require two mutexes to be locked
        std::shared_ptr<T> try_pop() {
            if (!head) {
                return std::shared_ptr<T>();
            }

            std::shared_ptr<T> const res = std::make_shared<T>(std::move(head->data));
            std::unique_ptr<node> const old_head = std::move(head);
            head = std::move(old_head->next);
            return res;
        }

        void push(T new_value) {
            std::unique_ptr<node> p = std::make_unique<node>(std::move(new_value));
            node *const new_tail = p.get();

            if (tail) {
                tail->next = std::move(p);
            } else {
                head = std::move(p);
            }
            tail = new_tail;

        }
    };
}

/**
 * Invariants:
 * 1. tail->next == nullptr
 * 2. tail->data == nullptr (tail is always a dummy node!)
 * 3. head == tail => empty list
 * 4. Single elem list => head->next == tail
 * 5. For each node x in the list where x!=tail, x->data points to an instance of T and x->next points to next node.
 *    x->next == tail implies x is the last node in the list
 * 6. Following next nodes will eventually yield tail
 */
namespace concurrentLockNoWait {
    template<typename T>
    class queue {
    private:
        struct node {
            std::shared_ptr<T> data; //ptr instead of primitive, in order to allow pointing to a dummy node
            std::unique_ptr<node> next;
        };

        std::mutex head_mutex;
        std::unique_ptr<node> head; //ownership. also: automatically deleted when no longer pointed to by anyone

        std::mutex tail_mutex;
        node *tail; //duplicate ptr to 'some' node

        //Wrapping functionality in functions to facilitate locking
        node *get_tail() {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            return tail;
        }

        std::unique_ptr<node> pop_head() {
            std::lock_guard<std::mutex> head_lock(head_mutex);

            //Lock tail for the duration of this check (in get_tail())
            if (head.get() == get_tail()) {
                return nullptr;
            }
            std::unique_ptr<node> old_head = std::move(head);
            head = std::move(old_head->next);
            return old_head;
        }

    public:
        //dummy node always in place; head is never NULL
        queue() :
                head(std::make_unique<node>()), tail(head.get()) {}

        queue(const queue &other) = delete;

        queue &operator=(const queue &other)= delete;

        /**
         * IMPORTANT: try_pop() and push() never operate on the same node!
         * The only case when this is (or rather, was) actually possible, is when head == tail
         * Such a case will now be protected in try_pop()
         */

        std::shared_ptr<T> try_pop() {
            std::unique_ptr<node> old_head = pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        void push(T new_value) {

            std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
            std::unique_ptr<node> p = std::make_unique<node>();

            node *const new_tail = p.get();

            //Lock tail
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            //given that there is ALWAYS a (dummy) node available, don't really need to manipulate head here
            tail->next = std::move(p);
            tail = new_tail;
        }
    };
}

namespace concurrentLockWait {
    //unbounded queue implementation
    template<typename T>
    class queue {
    private:
        struct node {
            std::shared_ptr<T> data; //ptr instead of primitive, in order to allow pointing to a dummy node
            std::unique_ptr<node> next;
        };

        std::mutex head_mutex;
        std::unique_ptr<node> head; //ownership. also: automatically deleted when no longer pointed to by anyone

        std::mutex tail_mutex;
        node *tail; //duplicate ptr to 'some' node

        std::condition_variable data_cond;

        //Wrapping functionality in functions to facilitate locking
        node *get_tail() {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            return tail;
        }

        std::unique_ptr<node> pop_head() {
            std::unique_ptr<node> old_head = std::move(head);
            head = std::move(old_head->next);
            return old_head;
        }

        std::unique_lock<std::mutex> wait_for_data()    {
            std::unique_lock<std::mutex> head_lock(head_mutex);
            data_cond.wait(head_lock,[&]{return head.get() != get_tail();});
            return std::move(head_lock);
        }

        std::unique_ptr<node> wait_pop_head() {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value) {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            value = std::move(*head->data);
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head() {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if(head.get() == get_tail())    {
                return std::unique_ptr<node>();
            }
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value) {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if(head.get() == get_tail())    {
                return std::unique_ptr<node>();
            }
            value = std::move(*head->data);
            return pop_head();
        }

    public:
        //dummy node always in place; head is never NULL
        queue() :
                head(std::make_unique<node>()), tail(head.get()) {}

        queue(const queue &other) = delete;

        queue &operator=(const queue &other)= delete;

        std::shared_ptr<T> wait_and_pop() {
            std::unique_ptr<node> const old_head = wait_pop_head();
            return old_head->data;
        }

        void wait_and_pop(T& value) {
            std::unique_ptr<node> const old_head = wait_pop_head(value);
        }

        /**
         * IMPORTANT: try_pop() and push() never operate on the same node!
         * The only case when this is (or rather, was) actually possible, is when head == tail
         * Such a case will now be protected in try_pop()
         */

        std::shared_ptr<T> try_pop() {
            std::unique_ptr<node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool try_pop(T& value)  {
            std::unique_ptr<node> const old_head = try_pop_head(value);
            return old_head ? true : false;
        }

        void push(T new_value) {

            std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
            std::unique_ptr<node> p = std::make_unique<node>();
            {
                //Lock tail
                std::lock_guard<std::mutex> tail_lock(tail_mutex);
                tail->data = new_data;
                node *const new_tail = p.get();
                //given that there is ALWAYS a (dummy) node available, don't really need to manipulate head here
                tail->next = std::move(p);
                tail = new_tail;
            }
            data_cond.notify_one();
        }

        bool empty()    {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            return (head.get() == get_tail());
        }
    };
}
#endif //SANDMP_QUEUE_H
