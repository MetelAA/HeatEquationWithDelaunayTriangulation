#include "AnalyticalSolution.h"

AnalyticalSolution::AnalyticalSolution(double width, double height, double thermal_conductivity_сoefficient,
                                       const DTO::TemperatureInitValues &t_init) : width(width),
    height(height),
    thermalConductivityСoefficient(thermal_conductivity_сoefficient),
    tInit(t_init) {
    this->C_mn = std::vector<std::vector<double> >(Constants::harmonicCountInNonStationarySeries,
                                                   std::vector<double>(Constants::harmonicCountInNonStationarySeries,
                                                                       0.0));
    compute_C_mn();
}

double AnalyticalSolution::compute_TInPointInTime(Point_2 p, double t) const {
    double x = p.getX(), y = p.getY();

    double rez = compute_TStationary(x, y); //значение температуры в точке (посчитанное значение ряда Фурье)
    for (int m = 1; m <= Constants::harmonicCountInNonStationarySeries; ++m) {
        for (int n = 1; n <= Constants::harmonicCountInNonStationarySeries; ++n) {
            double alpha = std::pow(m * Constants::PI / this->width, 2) + std::pow(n * Constants::PI / this->height, 2);
            rez += C_mn[m - 1][n - 1] * std::sin(m * Constants::PI * x / this->width) *
                    std::sin(n * Constants::PI * y / this->height) *
                    exp(-this->thermalConductivityСoefficient * alpha * t);
        }
    }

    return rez;
}

double AnalyticalSolution::sinh_l(double a) {
    if (a > 700.0) return 0;
    double rez = std::sinh(a);
    return (std::isfinite(rez) && rez > 1e-300) ? rez : 0.0;
}

double AnalyticalSolution::compute_TStationary(double x, double y) const {
    double T_st = 0;

    {
        //левая стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4 / (k * Constants::PI) * std::sin((k * Constants::PI * y) / this->height) *
            (
                (sinh_l((k * Constants::PI * (this->width - x)) / this->height))
                /
                (sinh_l((k * Constants::PI * this->width) / this->height))
            );
        }
        T_st += tInit.leftBoundaryT * s;
    }

    {
        //правая стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4 / (k * Constants::PI) * std::sin((k * Constants::PI * y) / this->height) *
            (
                (sinh_l(k * Constants::PI * x / this->height))
                /
                (sinh_l(k * Constants::PI * this->width / this->height))
            );
        }
        T_st += tInit.rightBoundaryT * s;
    }

    {
        //нижняя стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4 / (k * Constants::PI) * std::sin((k * Constants::PI * x) / this->width) *
            (
                (sinh_l(k * Constants::PI * (this->height - y) / this->width))
                /
                (sinh_l(k * Constants::PI * this->height / this->width))
            );
        }
        T_st += tInit.bottomBoundaryT * s;
    }

    {
        //верхняя стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4 / (k * Constants::PI) * std::sin((k * Constants::PI * x) / this->width) *
            (
                (sinh_l(k * Constants::PI * y / this->width))
                /
                (sinh_l(k * Constants::PI * this->height / this->width))
            );
        }
        T_st += tInit.topBoundaryT * s;
    }

    return T_st;
}

void AnalyticalSolution::compute_C_mn() {
    double dx, dy;
    dx = this->width / Constants::quadraturePoints;
    dy = this->height / Constants::quadraturePoints;

    for (int m = 1; m <= Constants::harmonicCountInNonStationarySeries; ++m) {
        for (int n = 1; n <= Constants::harmonicCountInNonStationarySeries; ++n) {
            double acc = 0;
            for (int i = 0; i < Constants::quadraturePoints; ++i) {
                double xi = (i + 0.5) * dx;
                double sinX = std::sin(m * Constants::PI * xi / this->width);
                for (int j = 0; j < Constants::quadraturePoints; ++j) {
                    double yj = (j + 0.5) * dy;
                    acc += (tInit.plateT0 - compute_TStationary(xi, yj)) * sinX * std::sin(
                        n * Constants::PI * yj / this->height) * dx * dy;
                }
            }
            C_mn[m - 1][n - 1] = 4.0 / (this->width * this->height) * acc;
        }
    }
}
