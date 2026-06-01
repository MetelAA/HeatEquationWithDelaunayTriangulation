
#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <stdexcept>
#include <utility>
#include <vector>
#include "Triangulation/Point_2.h"

namespace DTO{
    struct TriangleFace {
        TriangleFace(size_t v1Index, size_t v2Index, size_t v3Index) : v1_index(v1Index), v2_index(v2Index),
                                                                       v3_index(v3Index) {}

        TriangleFace() = default;
        TriangleFace(const std::vector<size_t>& indexes) {
            if (indexes.size() != 3)
                throw std::runtime_error("indexes vector for TriangleFace size must be equal 3");
            v1_index = indexes[0];
            v2_index = indexes[1];
            v3_index = indexes[2];
        }

        size_t v1_index, v2_index, v3_index; //индексы точек в передаваемом векторе координат


        template<class Archive>
        void serialize(Archive & archive) {
            archive(v1_index, v2_index, v3_index);
        }
    };



    struct TriangulationResult{
        TriangulationResult(const std::vector<Point_2> &points, const std::vector<DTO::TriangleFace> &faces) : points(points), faces(faces){}

        std::vector<Point_2> points;
        std::vector<DTO::TriangleFace> faces;
    };

    struct TemperatureInitValues {
        const double plateT0, leftBoundaryT, rightBoundaryT, topBoundaryT, bottomBoundaryT;

        TemperatureInitValues(double plate_t0, double left_boundary_t, double right_boundary_t, double top_boundary_t,
            double bottom_boundary_t)
            : plateT0(plate_t0),
              leftBoundaryT(left_boundary_t),
              rightBoundaryT(right_boundary_t),
              topBoundaryT(top_boundary_t),
              bottomBoundaryT(bottom_boundary_t) {
        }
    };

    struct PlateParams {//width и height даётся в метрах, так же как и все остальные физически величины даются в системе СИ
        const double width, height, thermalConductivityCoefficient;
        TemperatureInitValues tInit;

        PlateParams(double width, double height, double thermal_conductivity_coefficient,
            const TemperatureInitValues &t_init)
            : width(width),
              height(height),
              thermalConductivityCoefficient(thermal_conductivity_coefficient),
              tInit(t_init) {
        }
    };

    struct HeatPoint {
        Point_2 point;
        double temp;

        HeatPoint() = default;
        HeatPoint(const Point_2 &point, double temp)
            : point(point),
              temp(temp) {
        }

        template<class Archive>
        void serialize(Archive & archive) {
            archive(point, temp);
        }
    };

    struct HeatEquationStepResult {
        HeatEquationStepResult() = default;
        HeatEquationStepResult(std::vector<HeatPoint> points, const std::vector<DTO::TriangleFace> &faces)
            : points(std::move(points)),
              faces(faces) {
        }

        std::vector<HeatPoint> points;
        std::vector<DTO::TriangleFace> faces;

        template<class Archive>
        void serialize(Archive & archive) {
            archive(points, faces);
        }
    };

    struct StepResultAndInfoForWrite {
        HeatEquationStepResult result;
        double curTime;
        int frameNumber;

        StepResultAndInfoForWrite() = default;
        StepResultAndInfoForWrite(const HeatEquationStepResult &result, double cur_time, int step_number)
            : result(result),
              curTime(cur_time),
              frameNumber(step_number) {
        }

        template<class Archive>
        void serialize(Archive & archive) {
            archive(result, curTime, frameNumber);
        }
    };



    class AllDataResult {
    public:
        AllDataResult() = default;

        AllDataResult(double width, double height, double maxT, double minT, int common_steps, double experiment_time, double dt,
                      const std::vector<StepResultAndInfoForWrite> &steps_results)
            : width(width), height(height), maxT(maxT), minT(minT), experimentTime(experiment_time),
              dt(dt),
              commonFrames(common_steps),
              stepsResults(steps_results) {
        }

        int common_frames() const {
            return commonFrames;
        }

        double experiment_time() const {
            return experimentTime;
        }

        double get_dt() const {
            return dt;
        }

        double max_t() const {
            return maxT;
        }

        double min_t() const {
            return minT;
        }

        double experiment_time1() const {
            return experimentTime;
        }


        std::vector<StepResultAndInfoForWrite> steps_results() const {
            return stepsResults;
        }

        template<class Archive>
            void serialize(Archive & archive) {
            archive(width, height, maxT, minT, commonFrames, experimentTime, dt, stepsResults);
        }

    private:
        double width, height, maxT, minT, experimentTime, dt;
        int commonFrames;
        std::vector<StepResultAndInfoForWrite> stepsResults;
    };

}



#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DTO_H
