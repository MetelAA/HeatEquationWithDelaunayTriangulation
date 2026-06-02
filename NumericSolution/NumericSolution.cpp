
#include "NumericSolution.h"

#include <random>

#include "../Triangulation/DelaunayTriangulation.h"

NumericSolution::NumericSolution(int vertex_count, const DTO::PlateParams &plateParams, double dt) : height(plateParams.height),
      width(plateParams.width),
      thermalConductivityCoefficient(plateParams.thermalConductivityCoefficient),
      tInit(plateParams.tInit) {
    //создадим сетку (для начала границы потом натыкаем рандомно точек внутри)
    int boundaryVertexesCount = vertex_count * Constants::vertexOnBorderToAllVertexPropotion;
    boundaryVertexesCount = boundaryVertexesCount % 2 == 1 ? boundaryVertexesCount+1 : boundaryVertexesCount;
    int internalVertexesCount = vertex_count - boundaryVertexesCount;

    this->points.reserve(vertex_count+3); //мало ли добили до чётности

    std::vector<FiniteElementMethodHeatEquation::BoundaryNode> boundaryNodes;
    {
        int S = boundaryVertexesCount / 2;
        int K = std::round(S * (width / (width+height))); // K - кол-во интервалов на стороне a (a у нас будет нижней/верхней стороной, т.е. width)
        int M = S - K; // M - кол-во интервалов на правой/левой стороне (height)

        K = K == 0 ? 1 : K;//на всякий случай чтобы с делением на 0 не упасть
        M = M == 0 ? 1 : M;

        double da = width / K;
        double db = height / M;

        {
            double sum = da+db; //нужна для приведения температуры к среднему значение исходя из "близости" к двум граням

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


        //теперь рандомно раскидаем точки, но сделаем отступ минимальный отступ от границ, пусть будет (da+db)/25, подгоночное значение

        double fromBoundaryMargin = (da+db) / 25;
        if (width - fromBoundaryMargin <= 0 || height - fromBoundaryMargin <= 0) { //на всякий случай
            fromBoundaryMargin = 0;
        }

        std::mt19937 gen{std::random_device{}()};
        std::uniform_real_distribution<double> distX(fromBoundaryMargin, width - fromBoundaryMargin);
        std::uniform_real_distribution<double> distY(fromBoundaryMargin, height - fromBoundaryMargin);

        //рандомно раскидываем точки на плоскости
        for (int i = 0; i < internalVertexesCount; ++i)
            points.emplace_back(distX(gen), distY(gen));
    }


    std::cout << "There are " << points.size() << " vertexes \n";
    Triangulation::DelaunayTriangulation triangulation(points);
    DTO::TriangulationResult rez = triangulation.getTriangulationResult();

    points = rez.points;
    faces = rez.faces;


    this->heatEquation = FiniteElementMethodHeatEquation(points, faces, boundaryNodes, tInit.plateT0, dt, thermalConductivityCoefficient);

}

const std::vector<Point_2> & NumericSolution::getPoints() const {
    return this->points;
}

const std::vector<DTO::TriangleFace> & NumericSolution::getFaces() const {
    return this->faces;
}

DTO::HeatEquationStepResult NumericSolution::step() {
    std::vector<DTO::HeatPoint> heat_points;
    this->heatEquation.step();
    FiniteElementMethodHeatEquation::Nodes nodes = this->heatEquation.getNodesDataAccessInterface();

    for (int i = 0; i < nodes.getNodesSize(); ++i) {
        FiniteElementMethodHeatEquation::Node node = nodes.getNode(i);
        heat_points.emplace_back(node.coords, node.temp);
    }
    return {heat_points, this->faces};
}



double NumericSolution::getCurrentTime() const {
    return this->heatEquation.getCurrentTime();
}

DTO::HeatEquationStepResult NumericSolution::getZeroFrame() const {
    std::vector<DTO::HeatPoint> heat_points;
    FiniteElementMethodHeatEquation::Nodes nodes = this->heatEquation.getNodesDataAccessInterface();

    for (int i = 0; i < nodes.getNodesSize(); ++i) {
        FiniteElementMethodHeatEquation::Node node = nodes.getNode(i);
        heat_points.emplace_back(node.coords, node.temp);
    }
    return {heat_points, this->faces};
}


