//
// Created by manolee on 3/23/18.
//
#include "quicksort.h"

std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list)
{
    for (auto &i : list) {
        ostr << " " << i;
    }
    return ostr;
}
