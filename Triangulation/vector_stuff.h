#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H

#include "Point_2.h"

namespace vector_s{
    enum class orientation{
        left, right, collinear
    };


    inline orientation vector_orientation(const Point_2& a, const Point_2& b, const Point_2& c){
        Point_2 ab = b - a;
        Point_2 ac = c - a;
        double ps_scal = Point_2::pseudo_scalar(ab, ac);
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
    inline bool in_circle(const Point_2& a, const Point_2& b, const Point_2& c, const Point_2& d) {
        double adx = a.getX() - d.getX(), ady = a.getY() - d.getY();
        double bdx = b.getX() - d.getX(), bdy = b.getY() - d.getY();
        double cdx = c.getX() - d.getX(), cdy = c.getY() - d.getY();

        double a_sq = a.getX()*a.getX() + a.getY()*a.getY() - d.getX()*d.getX() - d.getY()*d.getY();
        double b_sq = b.getX()*b.getX() + b.getY()*b.getY() - d.getX()*d.getX() - d.getY()*d.getY();
        double c_sq = c.getX()*c.getX() + c.getY()*c.getY() - d.getX()*d.getX() - d.getY()*d.getY();

        double det = adx * (bdy * c_sq - b_sq * cdy)
                     - ady * (bdx * c_sq - b_sq * cdx)
                     + a_sq * (bdx * cdy - bdy * cdx);

        const double eps = 1e-12;
        return det > eps;   // строго внутри
    }
}

#endif