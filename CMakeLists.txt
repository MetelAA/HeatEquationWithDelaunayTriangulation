cmake_minimum_required(VERSION 3.16)
project(HeatEquationWithDelaunayTriangulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# --- Подключение cereal (header-only, без Boost) ---
include(FetchContent)

# Принудительно отключаем сборку тестов и поиск Boost
set(JUST_INSTALL_CEREAL ON CACHE BOOL "Skip building tests and sandbox" FORCE)

FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal.git
        GIT_TAG        v1.3.2
)
FetchContent_MakeAvailable(cereal)

# --- Исполняемый файл ---
add_executable(HeatEquationWithDelaunayTriangulation
        main.cpp
        Triangulation/point_2.h
        Triangulation/vector_stuff.h
        Triangulation/dcel.cpp
        Triangulation/dcel.h
        Triangulation/DelaunayTree.cpp
        Triangulation/DelaunayTree.h
        Triangulation/DelaunayTriangulation.cpp
        Triangulation/DelaunayTriangulation.h
        magic_constants.h
        NumericSolution/FiniteElementMethod/FiniteElementMethodHeatEquation.cpp
        NumericSolution/FiniteElementMethod/FiniteElementMethodHeatEquation.h
        DTO.h
        NumericSolution/NumericSolution.cpp
        NumericSolution/NumericSolution.h
        AnalyticalSolution/AnalyticalEquation.cpp
        AnalyticalSolution/AnalyticalEquation.h
        SolutionController.cpp
        SolutionController.h
        AnalyticalSolution/AnalyticalSolution.cpp
        AnalyticalSolution/AnalyticalSolution.h
        ReadWriteFiles/cereal_f.h
)

target_link_libraries(HeatEquationWithDelaunayTriangulation PRIVATE cereal)