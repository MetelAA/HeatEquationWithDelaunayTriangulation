//
// Created by Artem on 28.05.2026.
//

#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H


#include <utility>
#include <vector>
#include <stdexcept>
#include "../Triangulation/Point_2.h"
#include "../Triangulation/DCEL.h"
#include "../Triangulation/DTO.h"

class FiniteElementMethodHeatEquation {
public:
    struct Node{
    public:
        Node(const Point_2 &coords, double temp) : coords(coords), temp(temp) {}
        Point_2 coords;
        double temp;
    };

    class Nodes{
    public:
        Nodes() {}

        Nodes(std::vector<Point_2> coords, std::vector<double> actualTemp) : coords(std::move(coords)),
                                                                                                        actualTemp(std::move(
                                                                                                                actualTemp)) {
            temporaryTempVector = std::vector<double>(actualTemp.size());
        }
        Node getNode(size_t index) const {
            if (index < 0 || index >= coords.size())
                throw std::runtime_error("Finite Element node must be between 0 and nodes.size()");
            return {coords[index], actualTemp[index]};
        }

        void setNextStepTemp(size_t index, double temp){
            if (index < 0 || index >= coords.size())
                throw std::runtime_error("Finite Element node must be between 0 and nodes.size()");
            temporaryTempVector[index] = temp;
        }

        void changeTemporaryToMain(){
            actualTemp = temporaryTempVector;
        }

    private:
        std::vector<Point_2> coords;
        std::vector<double> actualTemp;
        std::vector<double> temporaryTempVector;
    };

    FiniteElementMethodHeatEquation(const std::vector<Point_2>& points, const std::vector<DTO::Face>& faces, const std::vector<DTO::BoundaryNode>& boundaryNodes) {

    }

private:
    Nodes nodes;
    std::vector<std::vector<double>> A;
    std::vector<double> M;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
