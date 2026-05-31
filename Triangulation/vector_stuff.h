#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H

#include "point_2.h"

namespace vector_s{
    enum class orientation{
        left, right, collinear
    };


    inline orientation vector_orientation(const point_2& a, const point_2& b, const point_2& c){
        point_2 ab = b - a;
        point_2 ac = c - a;
        double ps_scal = point_2::pseudo_scalar(ab, ac);
        if(std::abs(ps_scal) < 1e-9){
            return orientation::collinear;
        }else if(ps_scal > 0){
            return orientation::left;
        }else{
            return orientation::right;
        }
    }

    // a, b, c – вершины треугольника, перечисленные против часовой стрелки
    // d – проверяемая точка
    // true - если d лежит строго внутри окружности, описанной вокруг треугольника abc
    inline bool in_circle(const point_2& a, const point_2& b, const point_2& c, const point_2& d) {
        double adx = a.getX() - d.getX(), ady = a.getY() - d.getY();
        double bdx = b.getX() - d.getX(), bdy = b.getY() - d.getY();
        double cdx = c.getX() - d.getX(), cdy = c.getY() - d.getY();

        double a_sq = adx * adx + ady * ady;
        double b_sq = bdx * bdx + bdy * bdy;
        double c_sq = cdx * cdx + cdy * cdy;

        double det = adx * (bdy * c_sq - b_sq * cdy)
                     - ady * (bdx * c_sq - b_sq * cdx)
                     + a_sq * (bdx * cdy - bdy * cdx);

        const double eps = 1e-12;
        return det > eps;   // строго внутри
    }
}

#endif