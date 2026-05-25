#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_POINT_2_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_POINT_2_H

class Point_2{
public:
    Point_2(double x, double y) : x(x), y(y) {}

    Point_2 operator+ (const Point_2& other) const{ //this - левый операнд, other - правый
        return {this->x + other.x, this->y + other.y};
    }

    Point_2 operator- (const Point_2& other) const{
        return {this->x - other.x, this->y - other.y};
    }

    Point_2 operator* (double multiplier) const { // умножить на скаляр
        return {this->x * multiplier, this->y * multiplier};
    }

    static double pseudo_scalar(const Point_2& a, const Point_2& b){
        return a.x * b.y - a.y * b.x;
    }

    static double scalar(const Point_2& a, const Point_2& b){
        return a.x * b.x + a.y * b.y;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

protected:
    double x, y; //тут x и y напрямую зависят от введённых точек (размеров плоскости), т.е. все точки лежат в диапазоне от 0 до максимальной ширины/высоты


};

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_POINT_2_H