#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_VECTOR_STUFF_H

#include "Point_2.h"

namespace vector_s{
    enum class orientation{
        left, right, collinear
    };


    orientation vector_orientation(const Point_2& a, const Point_2& b, const Point_2& c){
        Point_2 ab = b - c;
        Point_2 ac = c - a;
        double ps_scal = Point_2::pseudo_scalar(ab, ac);
        if(ps_scal < 1e-9){
            return orientation::collinear;
        }else if(ps_scal > 0){
            return orientation::left;
        }else{
            return orientation::right;
        }
    }
}

#endif