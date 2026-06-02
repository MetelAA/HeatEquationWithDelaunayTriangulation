#include <chrono>
#include <iostream>
#include <thread>

#include "SolutionController.h"

int main() {
    DTO::TemperatureInitValues tInit(20, 40, 60, 80, 100);
    DTO::PlateParams params(0.1, 0.1, 9.71e-5, tInit);
    SolutionController controller(350, params, 0.05, 1000);


}
