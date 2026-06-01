
#include "NumericSolution.h"

#include <random>

#include "../Triangulation/DelaunayTriangulation.h"

NumericSolution::NumericSolution(int vertex_count, double thermalConductivityСoefficient, double width,
                                 double height, DTO::TemperatureInitValues tInit, double dt, double experimentTime) {
    //создадим сетку (для начала границы потом натыкаем рандомно точек внутри)
    int boundaryVertexesCount = vertex_count * Constants::vertexOnBorderToAllVertexPropotion;
    boundaryVertexesCount = boundaryVertexesCount % 2 == 1 ? boundaryVertexesCount+1 : boundaryVertexesCount;
    int internalVertexesCount = vertex_count - boundaryVertexesCount;

    this->points.reserve(vertex_count+1); //мало ли добили до чётности

    std::vector<DTO::BoundaryNode> boundaryNodes;
    {
        int S = boundaryVertexesCount / 2;
        int K = std::round(S * (width / (width+height))); // K - кол-во интервалов на стороне a (a у нас будет нижней/верхней стороной, т.е. width)
        int M = S - K; // M - кол-во интервалов на правой/левой стороне (height)

        K = K == 0 ? 1 : K;//на всякий случай чтобы с делением на 0 не упасть
        M = M == 0 ? 1 : M;

        double da = width / K;
        double db = height / M;

        {
            double sum = da+db; //нужна для приведения темперутры к среднему значение исходя из "близости" к двум грянм

            double leftT = tInit.leftBoundaryT;
            double rightT = tInit.rightBoundaryT;
            double bottomT = tInit.bottomBoundaryT;
            double topT = tInit.topBoundaryT;
            //левая нижняя
            boundaryNodes.emplace_back(points.size(), (da * bottomT + db * leftT) / sum);
            points.emplace_back(0, 0);
            //правая нижняя
            boundaryNodes.emplace_back(points.size(), (da * bottomT + db * rightT) / sum);
            points.emplace_back(width, 0);
            //правая верхняя
            boundaryNodes.emplace_back(points.size(), (da * topT + db * rightT) / sum);
            points.emplace_back(width, height);
            //левая верхняя
            boundaryNodes.emplace_back(points.size(), (da * topT + db * leftT) / sum);
            points.emplace_back(0, height);
        }

        //начнём с нижней/верхней границы
        for (int i = 1; i <= K - 1; ++i) {
            //нижняя граница
            boundaryNodes.emplace_back(points.size(), tInit.bottomBoundaryT);
            points.emplace_back(i * da, 0.0);
            //верхняя граница
            boundaryNodes.emplace_back(points.size(), tInit.topBoundaryT);
            points.emplace_back(i * da, height);
        }
        //теперь правая/левая
        for (int i = 1; i <= M-1; ++i) {
            //левая граница
            boundaryNodes.emplace_back(points.size(), tInit.leftBoundaryT);
            points.emplace_back(0, i * db);
            //правая граница
            boundaryNodes.emplace_back(points.size(), tInit.rightBoundaryT);
            points.emplace_back(width, i * db);
        }


        //теперь рандомно раскидаем точки, но сделаем отступ минимальный отступ от границ, пусть будет (da+db)/10 подгоночное значение

        double fromBoundaryMargin = (da+db) / 25;
        if (width - fromBoundaryMargin <= 0 || height - fromBoundaryMargin <= 0) { //на всякий случай, убрать полсе тестов
            fromBoundaryMargin = 0;
        }

        std::mt19937 gen{std::random_device{}()};
        std::uniform_real_distribution<double> distX(fromBoundaryMargin, width - fromBoundaryMargin);
        std::uniform_real_distribution<double> distY(fromBoundaryMargin, height - fromBoundaryMargin);

        for (int i = 0; i < internalVertexesCount; ++i)
            points.emplace_back(distX(gen), distY(gen));
    }


    std::cout << "There are " << points.size() << " vertexes \n";
    Triangulation::DelaunayTriangulation triangulation(points);
    DTO::TriangulationResult res = triangulation.getTriangulationResult();

    points = res.points;

    std::cout << points.size() << "\n";
    std::cout << res.faces.size() << "\n";
    for(DTO::TriangleFace face : res.faces){
        std::cout << "triangle: " << "v1: x |" << points[face.v1_index].getX() << "|, y |" << points[face.v1_index].getY() << "|; " << "v2: x |" << points[face.v2_index].getX() << "|, y |" << points[face.v2_index].getY() << "|; "
        << "v3: x |" << points[face.v3_index].getX() << "|, y |" << points[face.v3_index].getY() << "|\n";
    }


    this->heatEquation = FiniteElementMethodHeatEquation(points, res.faces, boundaryNodes, tInit.plateT0, dt, thermalConductivityСoefficient);

    //сам эксперимент
    int stepsCount = std::ceil(experimentTime/dt);
    for (int i = 0; i < stepsCount; ++i) {
        this->heatEquation.step();

    }



}
