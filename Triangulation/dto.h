
#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H

#include <cstdio>
#include <stdexcept>
#include <vector>

namespace dto{
    struct TriangleFace {
        TriangleFace(size_t v1Index, size_t v2Index, size_t v3Index) : v1_index(v1Index), v2_index(v2Index),
                                                                       v3_index(v3Index) {}


        TriangleFace(const std::vector<size_t>& indexes) {
            if (indexes.size() != 3)
                throw std::runtime_error("indexes vector for TriangleFace size must be equal 3");
            v1_index = indexes[0];
            v2_index = indexes[1];
            v3_index = indexes[2];
        }

        size_t v1_index, v2_index, v3_index; //индексы точек в передаваемом векторе координат

    };

    struct BoundaryNode {
        size_t node_index; // индекс точки в передаваемом векторе координат
        double temp; // температура границы

        BoundaryNode(size_t nodeIndex, double temp) : node_index(nodeIndex), temp(temp) {}
    };

    struct TriangulationResult{
        TriangulationResult(const std::vector<point_2> &points, const std::vector<dto::TriangleFace> &faces) : points(points), faces(faces){}

        std::vector<point_2> points;
        std::vector<dto::TriangleFace> faces;
    };

}



#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
