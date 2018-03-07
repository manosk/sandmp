//
// Created by manolee on 3/7/18.
//

#include "readerwriter_mutex.h"

dns_entry dns_cache::find_entry(std::string const& domain) const
{
    //Obtain shared (read) lock!
    boost::shared_lock<boost::shared_mutex> lk(entry_mutex);
    auto const it = entries.find(domain);
    return (it == entries.end()) ? dns_entry() : it->second;
}
void dns_cache::update_or_add_entry(std::string const& domain,
                         dns_entry const& dns_details)
{
    //Obtain exclusive (write) lock!
    std::lock_guard<boost::shared_mutex> lk(entry_mutex);
    entries[domain] = dns_details;
}
