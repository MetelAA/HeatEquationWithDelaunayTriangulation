#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H

#include "DCEL.h"
#include "DelaunayTree.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unordered_map>


namespace Triangulation{
    class DelaunayTriangulation{
    public:
        DelaunayTriangulation(std::vector<Point_2> &pts);

        DTO::TriangulationResult getTriangulationResult();
        //содержит мапу, где храняться все ребра образующие границу, т.к. ребра у нас направленные,
        //то содержит обход границы против часовой стрелки, ключом мапы выступает исток ребра, значением - сток

    private:
        void validateEdge(DCEL::EdgeWrapper e);
        bool isEdgeInvalid(DCEL::EdgeWrapper e);
        void triangulate();
        DCEL dcel;
        DelaunayTree tree;
        std::vector<Point_2> points; //нужно в конструкторе найти самую большую, поставить её на первое место, остальных перемешать

    };
}

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TRIANGULATION_H
