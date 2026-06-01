#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H

#include "../Triangulation/DTO.h"
#include <complex>
#include "../magic_constants.h"
#include "../Triangulation/Point_2.h"

class AnalyticalSolution {
public:
    AnalyticalSolution(double width, double height, double thermal_conductivity_сoefficient,
        const DTO::TemperatureInitValues &t_init);

    double compute_TInPointInTime(Point_2 p, double t) const;

private:
    double compute_TStationary(double x, double y) const;
    void compute_C_mn();

    static double sinh_l(double a); //типо может переполниться и сломать (не уверен что правда)

    double width, height, thermalConductivityСoefficient;
    DTO::TemperatureInitValues tInit;
    std::vector<std::vector<double>> C_mn;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_ANALYTICALSOLUTION_H