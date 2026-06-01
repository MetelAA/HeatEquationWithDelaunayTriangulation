#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALEQUATION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALEQUATION_H

#include "../DTO.h"
#include <complex>
#include "../magic_constants.h"
#include "../Triangulation/Point_2.h"

class AnalyticalEquation {
public:
    AnalyticalEquation(const DTO::PlateParams& params);

    double compute_TInPointInTime(const Point_2& p, double t) const;

private:
    double compute_TStationary(double x, double y) const;
    void compute_C_mn();

    double sinh_ratio(double a, double b) const;

    double width, height, thermalConductivityCoefficient;
    DTO::TemperatureInitValues tInit;
    std::vector<std::vector<double>> C_mn;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALEQUATION_H