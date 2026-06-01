cmake_minimum_required(VERSION 3.16)
project(HeatEquationWithDelaunayTriangulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
add_executable(HeatEquationWithDelaunayTriangulation main.cpp Triangulation/point_2.h Triangulation/vector_stuff.h Triangulation/dcel.cpp Triangulation/dcel.h Triangulation/DelaunayTree.cpp Triangulation/DelaunayTree.h Triangulation/DelaunayTriangulation.cpp Triangulation/DelaunayTriangulation.h magic_constants.h NumericSolution/FiniteElementMethod/FiniteElementMethodHeatEquation.cpp NumericSolution/FiniteElementMethod/FiniteElementMethodHeatEquation.h Triangulation/dto.h
        NumericSolution/NumericSolution.cpp
        NumericSolution/NumericSolution.h
        AnalyticalSolution/AnalyticalSolution.cpp
        AnalyticalSolution/AnalyticalSolution.h)