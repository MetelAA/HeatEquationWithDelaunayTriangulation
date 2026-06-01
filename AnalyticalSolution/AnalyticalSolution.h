//
// Created by Artem on 01.06.2026.
//

#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H
#include <vector>


#include "AnalyticalEquation.h"
class Point_2;

class AnalyticalSolution {
public:
    AnalyticalSolution(const std::vector<Point_2> &points, const std::vector<DTO::TriangleFace>& faces, const DTO::PlateParams& params)
        : points(points), faces(faces), analytical_equation(params){
    }

    DTO::HeatEquationStepResult stepOn(double time);

private:
    const std::vector<Point_2>& points;
    const std::vector<DTO::TriangleFace>& faces;
    AnalyticalEquation analytical_equation;

};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H