#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DATAWRITERFORPLOTS_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DATAWRITERFORPLOTS_H
#include <fstream>
#include <stdexcept>
#include <vector>

#include "../magic_constants.h"

namespace DataWriter {
    static void writeTwoColumns(const std::string fileName, const std::vector<double>& times, const std::vector<double>& temperatures) {
        if (times.size() != temperatures.size())
            throw std::runtime_error("vectors must have the same size");
        std::ofstream file(fileName, std::ios::trunc);
        file.precision(15);
        for (size_t i = 0; i < times.size(); ++i) {
            file << times[i] << " " << temperatures[i] << "\n";
        }
    }
}

#endif