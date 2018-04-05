//
// Created by manolee on 4/5/18.
//

#ifndef SANDMP_LIST_H
#define SANDMP_LIST_H

#include <mutex>
#include <memory>

namespace threadsafe {
    template <typename T>
    class list
    {
    private:
        struct node
        {
            std::mutex m; //mutex PER NODE - pretty finegrained
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;

            node():
                    next()
            {}

            //cp
            node(T const& value):
                    data(std::make_shared<T>(value))
            {}
        };

        node head; //default-constructed dummy node; does not hold actual data

    public:
        list()=default;

        ~list()
        {
            remove_if([](node const&){return true;});
        }

        list(list const& other)=delete;
        list& operator=(list const& other)=delete;

        void push_front(T const& value)
        {
            std::unique_ptr<node> new_node = std::make_unique<node>(value);
            std::lock_guard<std::mutex> lk(head.m);
            new_node->next = std::move(head.next);
            head.next = std::move(new_node);
        }

        template<typename Function>
        void for_each(Function f)
        {
            node *current = &head;
            std::unique_lock<std::mutex> lk(head.m);
            //Note that we are not serving out the (dummy) head
            while(node* const next = current->next.get())
            {
                //Get next lock
                std::unique_lock<std::mutex> next_lk(next->m);
                //Unlock current one
                lk.unlock();
                //Launch function
                f(*next->data);
                //Update pointers
                current = next;
                //Move (still locked) next_lk to lk object
                lk = std::move(next_lk);
            }
        }

        template<typename Predicate>
        std::shared_ptr<T> find_first_if(Predicate p)
        {
            node *current = &head;
            std::unique_lock<std::mutex> lk(head.m);
            while(node* const next = current->next.get())
            {
                //Get next lock
                std::unique_lock<std::mutex> next_lk(next->m);
                //Unlock current one
                lk.unlock();
                //Evaluate predicate
                if(p(*next->data))
                {
                    return next->data;
                }
                //Update pointers
                current = next;
                //Move (still locked) next_lk to lk object
                lk = std::move(next_lk);
            }
            return std::shared_ptr<T>();
        }

        template<typename Predicate>
        void remove_if(Predicate p)
        {
            node *current = &head;
            std::unique_lock<std::mutex> lk(head.m);
            while(node* const next = current->next.get())
            {
                //Get next lock
                std::unique_lock<std::mutex> next_lk(next->m);
                //Unlock current one
                lk.unlock();
                //Evaluate predicate
                if(p(*next->data))
                {
                    //old_next: ptr to the one to be removed (once it goes out of scope)
                    std::unique_ptr<node> old_next = std::move(current->next);
                    current->next = std::move(next->next);
                    next_lk.unlock();
                }
                else
                {
                    lk.unlock();
                    //Update pointers
                    current = next;
                    //Move (still locked) next_lk to lk object
                    lk = std::move(next_lk);
                }
            }
        }
    };
}
#endif //SANDMP_LIST_H
