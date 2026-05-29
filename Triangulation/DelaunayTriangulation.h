#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTRIANGULATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTRIANGULATION_H

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

        void triangulate();
    
    private:
        void validateEdge(DCEL::EdgeWrapper e);
        bool isEdgeInvalid(DCEL::EdgeWrapper e);

        DCEL dcel;
        DelaunayTree tree;
        std::vector<Point_2> points; //нужно в конструкторе найти самую большую, поставить её на первое место, остальных перемешать

    };
}

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTRIANGULATION_H
