#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H

#include <cstdio>
#include <limits>

namespace Constants{

    static const size_t p_inf_left_top_index = -1; //p с индексом -1
    static const size_t p_inf_right_bottom_index = -2;
    static const size_t invalid_twin_edge = std::numeric_limits<size_t>::max();
}

#endif
