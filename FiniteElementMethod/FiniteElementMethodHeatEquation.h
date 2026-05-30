//
// Created by Artem on 28.05.2026.
//

#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H


#include <utility>
#include <vector>
#include <stdexcept>
#include <cmath>
#include "../Triangulation/point_2.h"
#include "../Triangulation/dcel.h"
#include "../Triangulation/dto.h"

class FiniteElementMethodHeatEquation {
public:
    struct Node{
    public:
        Node(const point_2 &coords, double temp, bool isBoundary) : coords(coords), temp(temp), isBoundary(isBoundary) {}
        point_2 coords;
        double temp;
        bool isBoundary;
    };

    class Nodes{
    public:
        Nodes() {}

        Nodes(const std::vector<point_2>& coords, const std::vector<double>& actualTemp, const std::vector<bool>& isBoundary) { //isBoundary - вектора на N элементов, где только граничным элементам соответсует значение true
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

        std::vector<size_t> getBoundaryNodesIndexes(){
            return boundaryNodesIndexes;
        }

        size_t getNodesSize() const {
            return nodes.size();
        }

    private:
        std::vector<Node> nodes;
        std::vector<size_t> boundaryNodesIndexes;
    };

    FiniteElementMethodHeatEquation(const std::vector<point_2>& points,
            const std::vector<dto::TriangleFace>& faces,
            const std::vector<dto::BoundaryNode>& boundaryNodes,
            double initTemperature,
            double dt,
            double thermalConductivityCoefficient);

    void step();

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
