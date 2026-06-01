#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_CEREAL_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_CEREAL_H

#include <fstream>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include "../DTO.h"

namespace cereal_f {
    static void writeToFile(const DTO::AllDataResult& data, const std::string& filename) {
        std::ofstream os(filename, std::ios::binary);
        cereal::BinaryOutputArchive archive(os);
        archive(data);
    }

    static DTO::AllDataResult readFromFile(const std::string& filename) {
        DTO::AllDataResult data;
        std::ifstream is(filename, std::ios::binary);
        cereal::BinaryInputArchive archive(is);
        archive(data);
        return data;
    }
}

#endif