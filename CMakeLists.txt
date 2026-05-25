cmake_minimum_required(VERSION 3.16)
project(HeatEquationWithDelaunayTriangulation)

set(CMAKE_CXX_STANDARD 17)

add_executable(HeatEquationWithDelaunayTriangulation main.cpp Point_2.h vector_stuff.h DCEL.cpp DCEL.h DelaunayTree.cpp DelaunayTree.h DelaunayTriangulation.cpp DelaunayTriangulation.h)