//
// Created by manolee on 3/7/18.
//

#ifndef SANDMP_READERWRITER_MUTEX_H
#define SANDMP_READERWRITER_MUTEX_H

#include <map>
#include <string>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>

//Example of use
class dns_entry {
public:
    std::string some_val;
};

class dns_cache {
    std::map<std::string,dns_entry> entries;
    mutable boost::shared_mutex entry_mutex;
public:
    dns_entry find_entry(std::string const& domain) const;
    void update_or_add_entry(std::string const& domain,
                             dns_entry const& dns_details);
};
#endif //SANDMP_READERWRITER_MUTEX_H
