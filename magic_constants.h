#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_MAGIC_CONSTANTS_H

#include <cstdio>
#include <limits>
#include <string>

namespace Constants{

    static const size_t p_inf = std::numeric_limits<size_t>::max()-1; //p с индексом -1
    static const double super = 1e7;
    static const double vertexOnBorderToAllVertexPropotion = 0.15;
    static const int numberOfMembersOfStationarySeries = 40; //число членов в ряде для стционарной части ур-я
    static const int harmonicCountInNonStationarySeries = 20; //число гармоник в нестационарной части ур-я, от него зависит кол-во C_mn = harmonicCountInNonStationarySeries^2
    static const int quadraturePoints = 30;
    static const double PI = 3.14159265358979323846;
    static const double writePerSecond = 0.05; //ровно столько раз за секунду будем записывать результат в память, столько же раз в секунду считаем аналитику
    static const std::string numericResultFile = "num_result.bin";
}

#endif
