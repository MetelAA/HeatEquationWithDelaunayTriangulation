#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_SOLUTIONCONTROLLER_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_SOLUTIONCONTROLLER_H
#include "AnalyticalSolution/AnalyticalSolution.h"
#include "NumericSolution/NumericSolution.h"
#include "ReadWriteFiles/cereal_f.h"

class SolutionController {
public:
    SolutionController(int vertex_count, const DTO::PlateParams& plateParams, double dt, double experimentTime) : plate_params(plateParams) {
        NumericSolution numeric_solution(vertex_count, plateParams, dt);
        AnalyticalSolution analytical_solution(numeric_solution.getPoints(), numeric_solution.getFaces(), plateParams);


        int timeForWrite = std::ceil(1 / Constants::writePerSecond);
        long stepsPerWrite = std::ceil(timeForWrite / dt);

        std::vector<DTO::StepResultAndInfoForWrite> numStepsResults;
        std::vector<DTO::StepResultAndInfoForWrite> analyticalStepsResults;

        std::vector<double> maxDiff; //будем считать max разницу между ЧИСЛЕННЫМ и аналитическим. т.е. численный-аналитический
        std::vector<double> RMSDiff; //яндекс говорит что RMS difference = Среднеквадратическая разница...

        long steps = 0;
        int frames = 1;
        {
            DTO::HeatEquationStepResult zeroFrame = numeric_solution.getZeroFrame();
            numStepsResults.push_back(DTO::StepResultAndInfoForWrite(zeroFrame, 0, 0));
            analyticalStepsResults.push_back(DTO::StepResultAndInfoForWrite(zeroFrame, 0, 0));
        }


        std::cout << "initialized successfully, start calc\n";
        while (numeric_solution.getCurrentTime() < experimentTime) {
            DTO::HeatEquationStepResult numStepResult = numeric_solution.step();
            if (steps % stepsPerWrite == 0) {
                DTO::HeatEquationStepResult analyticalStepResult = analytical_solution.stepOn(numeric_solution.getCurrentTime());
                numStepsResults.emplace_back(numStepResult, numeric_solution.getCurrentTime(), frames);
                analyticalStepsResults.emplace_back(analyticalStepResult, numeric_solution.getCurrentTime(), frames);

                maxDiff.push_back(getMaxDifference(numStepResult.points, analyticalStepResult.points));
                RMSDiff.push_back(getRMSDifference(numStepResult.points, analyticalStepResult.points));
                frames++;
            }
            steps++;
        }

        double maxT = this->plate_params.tInit.plateT0, minT = this->plate_params.tInit.plateT0;
        {
            maxT = std::max(maxT, this->plate_params.tInit.bottomBoundaryT);
            maxT = std::max(maxT, this->plate_params.tInit.topBoundaryT);
            maxT = std::max(maxT, this->plate_params.tInit.leftBoundaryT);
            maxT = std::max(maxT, this->plate_params.tInit.rightBoundaryT);
            minT = std::min(minT, this->plate_params.tInit.bottomBoundaryT);
            minT = std::min(minT, this->plate_params.tInit.topBoundaryT);
            minT = std::min(minT, this->plate_params.tInit.leftBoundaryT);
            minT = std::min(minT, this->plate_params.tInit.rightBoundaryT);
        }

        DTO::AllDataResult allDataNumResult(plateParams.width, plateParams.height, maxT, minT, frames, experimentTime, dt, numStepsResults);
        DTO::AllDataResult allDataAnalyticalResult(plateParams.width, plateParams.height, maxT, minT, frames, experimentTime, dt, analyticalStepsResults);

        cereal_f::writeToFile(allDataNumResult, Constants::numericResultFile);
        cereal_f::writeToFile(allDataAnalyticalResult, "analytical_result.bin");
    }

private:

    double getMaxDifference(const std::vector<DTO::HeatPoint>& num, const std::vector<DTO::HeatPoint>& analytic) {
        double maxDiff = INT_MIN;
        for (int i = 0; i < num.size(); i++) {
            if (num[i].point.getX() > 0 && num[i].point.getY() > 0 && num[i].point.getX() < plate_params.width && num[i].point.getY() < plate_params.height && num[i].point == analytic[i].point) {
                maxDiff = std::max(maxDiff, num[i].temp - analytic[i].temp);
            }
        }

        return maxDiff;
    }

    double getRMSDifference(const std::vector<DTO::HeatPoint>& num, const std::vector<DTO::HeatPoint>& analytic) {
        double acc = 0;
        int steps = 0;
        for (int i = 0; i < num.size(); i++) {
            if (num[i].point.getX() > 0 && num[i].point.getY() > 0 && num[i].point.getX() < plate_params.width && num[i].point.getY() < plate_params.height && num[i].point == analytic[i].point) {
                acc += std::pow(num[i].temp - analytic[i].temp, 2);
                steps++;
            }
        }

        acc = acc / steps;
        return std::sqrt(acc);
    }
    DTO::PlateParams plate_params;
};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_SOLUTIONCONTROLLER_H

// NUM avg step time: 0.000990623 s
// ANALYT step time: 0.0118762 s
//на 200 точках

// double totalTime = 0.0;

// auto t1 = std::chrono::steady_clock::now();
// auto t2 = std::chrono::steady_clock::now();
// totalTime += std::chrono::duration<double>(t2 - t1).count();
// ++steps;
// double avgTime = totalTime / steps;
// std::cout << "avg step time: " << avgTime << " s\n";