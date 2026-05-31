#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H

#include <cstdio>
#include <limits>

namespace Constants{

    static const size_t p_inf = std::numeric_limits<size_t>::max()-1; //p с индексом -1
    static const double super = 1e7;

}

#endif
