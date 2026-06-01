#include "AnalyticalEquation.h"

AnalyticalEquation::AnalyticalEquation(const DTO::PlateParams& params) : width(params.width),
                                                                         height(params.height),
                                                                         thermalConductivityCoefficient(params.thermalConductivityCoefficient),
                                                                         tInit(params.tInit) {
    this->C_mn = std::vector<std::vector<double> >(Constants::harmonicCountInNonStationarySeries,
                                                   std::vector<double>(Constants::harmonicCountInNonStationarySeries,
                                                                       0.0));
    compute_C_mn();
}

double AnalyticalEquation::compute_TInPointInTime(const Point_2& p, double t) const {
    double x = p.getX(), y = p.getY();

    double rez = compute_TStationary(x, y); //значение температуры в точке (посчитанное значение ряда Фурье)
    for (int m = 1; m <= Constants::harmonicCountInNonStationarySeries; ++m) {
        for (int n = 1; n <= Constants::harmonicCountInNonStationarySeries; ++n) {
            double alpha = std::pow(m * Constants::PI / this->width, 2) + std::pow(n * Constants::PI / this->height, 2);
            rez += C_mn[m - 1][n - 1] * std::sin(m * Constants::PI * x / this->width) *
                    std::sin(n * Constants::PI * y / this->height) *
                    exp(-this->thermalConductivityCoefficient * alpha * t);
        }
    }

    return rez;
}

double AnalyticalEquation::sinh_ratio(double a, double b) const {
    if (a > 600.0 || b > 600.0) {
        double exp_diff = std::exp(a - b);
        double correction = (1.0 - std::exp(-2.0 * a)) / (1.0 - std::exp(-2.0 * b));
        return exp_diff * correction;
    }
    double su = std::sinh(a);
    double sv = std::sinh(b);
    if (std::abs(sv) > 1e-300)
        return su / sv;
    return 0.0;
}

double AnalyticalEquation::compute_TStationary(double x, double y) const {
    double T_st = 0;

    {
        //левая стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4.0 / (k * Constants::PI) * std::sin((k * Constants::PI * y) / this->height) *
            (sinh_ratio(k * Constants::PI * (this->width - x) / this->height, k * Constants::PI * this->width / this->height));
        }
        T_st += tInit.leftBoundaryT * s;
    }

    {
        //правая стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4.0 / (k * Constants::PI) * std::sin((k * Constants::PI * y) / this->height) *
            (sinh_ratio(k * Constants::PI * x / this->height, k * Constants::PI * this->width / this->height));
        }
        T_st += tInit.rightBoundaryT * s;
    }

    {
        //нижняя стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4.0 / (k * Constants::PI) * std::sin((k * Constants::PI * x) / this->width) *
            (sinh_ratio(k * Constants::PI * (this->height - y) / this->width, k * Constants::PI * this->height / this->width));
        }
        T_st += tInit.bottomBoundaryT * s;
    }

    {
        //верхняя стенка
        double s = 0;
        for (int k = 1; k <= Constants::numberOfMembersOfStationarySeries; k += 2) {
            s += 4.0 / (k * Constants::PI) * std::sin((k * Constants::PI * x) / this->width) *
            (sinh_ratio(k * Constants::PI * y / this->width, k * Constants::PI * this->height / this->width));
        }
        T_st += tInit.topBoundaryT * s;
    }

    return T_st;
}

void AnalyticalEquation::compute_C_mn() {
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
