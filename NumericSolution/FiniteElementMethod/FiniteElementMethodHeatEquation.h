#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H


#include <utility>
#include <vector>
#include <stdexcept>
#include <cmath>
#include "../../Triangulation/Point_2.h"
#include "../../Triangulation/DCEL.h"
#include "../../DTO.h"

class FiniteElementMethodHeatEquation {
public:
    struct Node{
        Node(const Point_2 &coords, double temp, bool isBoundary) : coords(coords), temp(temp), isBoundary(isBoundary) {}
        Point_2 coords;
        double temp;
        bool isBoundary;
    };

    struct BoundaryNode {
        size_t node_index; // индекс точки в передаваемом векторе координат
        double temp; // температура границы

        BoundaryNode(size_t nodeIndex, double temp) : node_index(nodeIndex), temp(temp) {}
    };

    class Nodes{ //класс nodes сохраняет порядок как был в векторе points, т.е. индекс вершин в треугольниках в векторе faces остаются валидными!
    public:
        Nodes() {}

        Nodes(const std::vector<Point_2>& coords, const std::vector<double>& actualTemp, const std::vector<bool>& isBoundary) { //isBoundary - вектора на N элементов, где только граничным элементам соответсует значение true
            nodes.reserve(coords.size());
            for (int i = 0; i < coords.size(); ++i) {
                nodes.emplace_back(coords[i], actualTemp[i], isBoundary[i]);
                if (isBoundary[i])
                    boundaryNodesIndexes.push_back(i);
            }
        }


        const Node& getNode(size_t index) const {
            if (index < 0 || index >= nodes.size())
                throw std::runtime_error("Finite Element node must be between 0 and nodes.size()");
            return nodes[index];
        }

        void changeTempInNodes(const std::vector<double>& temp){
            for (int i = 0; i < nodes.size(); ++i) {
                nodes[i].temp = temp[i];
            }
        }

        const std::vector<size_t>& getBoundaryNodesIndexes() const{
            return boundaryNodesIndexes;
        }

        size_t getNodesSize() const {
            return nodes.size();
        }

    private:
        std::vector<Node> nodes;
        std::vector<size_t> boundaryNodesIndexes;
    };


    FiniteElementMethodHeatEquation() = default;

    FiniteElementMethodHeatEquation(const std::vector<Point_2>& points,
                                    const std::vector<DTO::TriangleFace>& trFaces,
                                    const std::vector<BoundaryNode>& boundaryNodes,
                                    double initTemperature,
                                    double dt,
                                    double thermalConductivityCoefficient);

    void step();

    double getCurrentTime() const;

    const Nodes& getNodesDataAccessInterface() const;
private:
    Nodes nodes;
    std::vector<std::vector<double>> L;
    std::vector<std::vector<double>> M;
    std::vector<std::vector<double>> A;
    double dt;
    double thermalConductivityCoefficient;
    double curTime;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
