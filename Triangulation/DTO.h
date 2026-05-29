
#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H

#include <cstdio>

namespace DTO{
    struct Face {
        size_t v1_index, v2_index, v3_index; //индексы точек в передаваемом векторе координат
    };

    struct BoundaryNode {
        size_t node_index; // индекс точки в передаваемом векторе координат
        double temp; // температура границы
    };
}



#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
