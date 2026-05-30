#include <iostream>
#include <optional>
#include "Triangulation/delaunay_triangulation.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    std::vector<point_2> allPoints = {
            point_2(20, 30),   // вершина границы
            point_2(60, 10),   // вершина границы
            point_2(90, 40),   // вершина границы
            point_2(80, 80),   // вершина границы
            point_2(40, 90),   // вершина границы
            point_2(10, 60),   // вершина границы
            point_2(40, 40),   // внутренняя
            point_2(60, 60),   // внутренняя
            point_2(30, 70),   // внутренняя
            point_2(70, 50)    // внутренняя
    };
    std::vector<point_2> boundary = {
            point_2(20, 30),
            point_2(60, 10),
            point_2(90, 40),
            point_2(80, 80),
            point_2(40, 90),
            point_2(10, 60)
    };
    std::unordered_map<point_2, point_2> boundaryMap = {
            { point_2(20,30), point_2(60,10) },
            { point_2(60,10), point_2(90,40) },
            { point_2(90,40), point_2(80,80) },
            { point_2(80,80), point_2(40,90) },
            { point_2(40,90), point_2(10,60) },
            { point_2(10,60), point_2(20,30) }
    };

    triangulation::delaunay_triangulation triangulation(allPoints);
    dto::TriangulationResult res = triangulation.getTriangulationResult(boundaryMap);

    std::vector<point_2> points = res.points;

    std::cout << points.size() << "\n";
    std::cout << res.faces.size();
    for(dto::TriangleFace face : res.faces){
        std::cout << "v1: x |" << points[face.v1_index].getX() << "|, y |" << points[face.v1_index].getY() << "; " << "v2: x |" << points[face.v2_index].getX() << "|, y |" << points[face.v2_index].getY() << "; "
        << "v3: x |" << points[face.v3_index].getX() << "|, y |" << points[face.v3_index].getY() << "\n";
    }


}
