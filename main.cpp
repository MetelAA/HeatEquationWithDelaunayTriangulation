#include <iostream>

#include "SolutionController.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    // std::vector<Point_2> allPoints = {
    //         Point_2(20, 30),   // вершина границы
    //         Point_2(60, 10),   // вершина границы
    //         Point_2(90, 40),   // вершина границы
    //         Point_2(80, 80),   // вершина границы
    //         Point_2(40, 90),   // вершина границы
    //         Point_2(10, 60),   // вершина границы
    //         Point_2(40, 40),   // внутренняя
    //         Point_2(60, 60),   // внутренняя
    //         Point_2(30, 70),   // внутренняя
    //         Point_2(70, 50)    // внутренняя
    // };
    // std::vector<Point_2> boundary = {
    //         Point_2(20, 30),
    //         Point_2(60, 10),
    //         Point_2(90, 40),
    //         Point_2(80, 80),
    //         Point_2(40, 90),
    //         Point_2(10, 60)
    // };




    // std::mt19937 gen{std::random_device{}()};
    // std::uniform_real_distribution<double> dist(0.1, std::nextafter(0.55, std::numeric_limits<double>::max()));
    //
    // int iterC = 1000;
    // double min = 1000000, max = -1, avg = 0;
    // for (int i = 0; i < iterC; ++i) {
    //     double generated = dist(gen);
    //     min = std::min(min, generated);
    //     max = std::max(max, generated);
    //     avg += generated;
    //     // std::cout << generated << "\n";
    // }
    // std::cout << "max: " << max << "  min: " << min << "  avg: " << (avg / iterC)<< "\n";


    DTO::TemperatureInitValues tInit(20, 40, 60, 80, 100);
    DTO::PlateParams params(0.5, 0.5, 9.71e-5, tInit);
    SolutionController controller(500, params, 0.05, 500);



}
