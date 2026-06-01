#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H

#include <iostream>
#include <vector>

#include "FiniteElementMethod/FiniteElementMethodHeatEquation.h"
#include "../DTO.h"
#include "../Triangulation/Point_2.h"
#include "FiniteElementMethod/FiniteElementMethodHeatEquation.h"

class NumericSolution {
public:
    NumericSolution(int vertex_count, const DTO::PlateParams& plateParams, double dt);

    const std::vector<Point_2>& getPoints() const;
    const std::vector<DTO::TriangleFace>& getFaces() const;
    DTO::HeatEquationStepResult step();
    double getCurrentTime() const;
    DTO::HeatEquationStepResult getZeroFrame() const;

private:
    std::vector<Point_2> points;
    std::vector<DTO::TriangleFace> faces;
    FiniteElementMethodHeatEquation heatEquation;
    double height, width, thermalConductivityCoefficient;
    DTO::TemperatureInitValues tInit;

};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_NUMERICSOLUTION_H