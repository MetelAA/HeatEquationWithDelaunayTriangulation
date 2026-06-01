//
// Created by Artem on 01.06.2026.
//

#include "AnalyticalSolution.h"

DTO::HeatEquationStepResult AnalyticalSolution::stepOn(double time) {
    std::vector<DTO::HeatPoint> heat_points;
    heat_points.reserve(this->points.size());

    for (const Point_2& p : this->points){
        heat_points.emplace_back(p, this->analytical_equation.compute_TInPointInTime(p, time));
    }
    return {heat_points, this->faces};
}


