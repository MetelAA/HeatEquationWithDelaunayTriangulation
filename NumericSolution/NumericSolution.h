#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H

#include <iostream>
#include <vector>

#include "FiniteElementMethod/FiniteElementMethodHeatEquation.h"
#include "../Triangulation/DTO.h"
#include "../Triangulation/Point_2.h"
#include "FiniteElementMethod/FiniteElementMethodHeatEquation.h"

class NumericSolution {
public:


    NumericSolution(int vertex_count, double thermalConductivityСoefficient, double width, double height, //width и height даётся в метрах, так же как и все остальные физически величины даются в системе СИ
    DTO::TemperatureInitValues tInit, double dt, double experimentTime
    );

private:
    std::vector<Point_2> points;
    FiniteElementMethodHeatEquation heatEquation;


};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H