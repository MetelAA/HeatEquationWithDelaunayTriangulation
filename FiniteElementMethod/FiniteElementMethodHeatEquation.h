//
// Created by Artem on 28.05.2026.
//

#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H


#include <utility>
#include <vector>
#include <stdexcept>
#include <cmath>
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

        size_t getNodesSize() const {
            return coords.size();
        }

    private:
        std::vector<Point_2> coords;
        std::vector<double> actualTemp;
        std::vector<double> temporaryTempVector;
    };

    FiniteElementMethodHeatEquation(const std::vector<Point_2>& points, const std::vector<DTO::Face>& faces, const std::vector<DTO::BoundaryNode>& boundaryNodes, double initTemperature, double dt, double thermalConductivityCoefficient) {
        //в этот конструктор попадают уже только внутренние грани, т.е. отсеяные от того что лишним образом натреагулировали
        //boundaryNodes - все граничные ноды, нужно где-то до этого напроецировать на них их температуры, не охота это здесь разводить учитывая что у нас есть список этих вершин на других, более ранних шагах
        this->dt = dt;
        this->thermalConductivityCoefficient = thermalConductivityCoefficient;

        //булем считать локальные матрицы для каждого элемента
        for(DTO::Face face : faces){
            std::vector<std::vector<double>> local_K;
            std::vector<std::vector<double>> local_M;
            {
                Point_2 i = points[face.v1_index];
                Point_2 j = points[face.v2_index];
                Point_2 k = points[face.v3_index];

                double s = i.getX() * (j.getY() - k.getY()) + j.getX() * (k.getY() - i.getY()) + k.getX() * (i.getY() - j.getY()); //площадь * 2

                double b_i, c_i; //vertex i, a_* - нет тк они откисают на этапе применения градиента
                b_i = j.getY() - k.getY();
                c_i = k.getX() - j.getX();

                double b_j, c_j;
                b_j = k.getY() - i.getY();
                c_j = i.getX() - k.getX();

                double b_k, c_k;
                b_k = i.getY() - j.getX();
                c_k = j.getX() - i.getX();


                {
                    double multiplier = 1.0 / (2*s);
                    local_K = {
                            {multiplier * (std::pow(b_i, 2) + std::pow(c_i, 2)), multiplier * (b_i*b_j + c_i*c_j), multiplier * (b_i*b_k + c_i*c_k) },
                            {multiplier * (b_j*b_i + c_j*c_i), multiplier * (std::pow(b_j, 2) + std::pow(c_j, 2)), multiplier * (b_j*b_k + c_j*c_k) },
                            {multiplier * (b_k*b_i + c_k*c_i), multiplier * (b_k*b_j + c_k*c_j), multiplier * (std::pow(b_k, 2) + std::pow(c_k, 2)) }
                    };
                }


                {
                    double m = s / 2.0 / 12.0; //multiplier
                    local_M = {
                            {m * 2, m, m},
                            {m, m * 2, m},
                            {m, m, 2 * m}
                    };
                }
            }

            //теперь складываем в глобальные матрицы

            std::vector<std::vector<double>> K(points.size(), std::vector<double>(points.size(), 0));
            M = std::vector<std::vector<double>>(points.size(), std::vector<double>(points.size(), 0));
            A = std::vector<std::vector<double>>(points.size(), std::vector<double>(points.size(), 0));
            std::vector<size_t> localIndexToGlobalIndex = {face.v1_index, face.v2_index, face.v3_index};

            for (int i = 0; i < local_K.size(); ++i) {
                for (int j = 0; j < local_K[0].size(); ++j) {
                    K[localIndexToGlobalIndex[i]][localIndexToGlobalIndex[j]] += local_K[i][j];
                    M[localIndexToGlobalIndex[i]][localIndexToGlobalIndex[j]] += local_M[i][j];
                }
            }

            for (int i = 0; i < local_K.size(); ++i) {
                for (int j = 0; j < local_K[0].size(); ++j) {
                    A[i][j] = M[i][j] + dt * thermalConductivityCoefficient * K[i][j];
                }
            }
        }

        std::vector<double> nodeTemperatures(points.size());
        for (int i = 0; i < nodeTemperatures.size(); ++i) {
            nodeTemperatures[i] = initTemperature;
        }
        for(DTO::BoundaryNode boundary : boundaryNodes){
            nodeTemperatures[boundary.node_index] = boundary.temp;
        }
        nodes = Nodes(points, nodeTemperatures);
    }

    void step(){
        
    }

private:
    Nodes nodes;
    std::vector<std::vector<double>> A;
    std::vector<std::vector<double>> M;
    double dt;
    double thermalConductivityCoefficient;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_FINITEELEMENTMETHODHEATEQUATION_H
