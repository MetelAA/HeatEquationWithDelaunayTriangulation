#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H

#include "dcel.h"
#include "delaunay_tree.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unordered_map>


namespace triangulation{
    class delaunay_triangulation{
    public:
        delaunay_triangulation(std::vector<point_2> &pts);

        dto::TriangulationResult getTriangulationResult(const std::unordered_map<point_2, point_2>& boundary_vertexes_map);
        //содержит мапу, где храняться все ребра образующие границу, т.к. ребра у нас направленные,
        //то содержит обход границы против часовой стрелки, ключом мапы выступает исток ребра, значением - сток

    private:
        void validateEdge(dcel::EdgeWrapper e);
        bool isEdgeInvalid(dcel::EdgeWrapper e);
        void triangulate();
        dcel dcel;
        delaunay_tree tree;
        std::vector<point_2> points; //нужно в конструкторе найти самую большую, поставить её на первое место, остальных перемешать

    };
}

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H
