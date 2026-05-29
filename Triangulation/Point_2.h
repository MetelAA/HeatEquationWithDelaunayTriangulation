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

    bool operator==(const Point_2& other) const {
        return x == other.x && y == other.y;
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

namespace std {
    template <>
    struct hash<Point_2> {
        std::size_t operator()(const Point_2& p) const {
            // простой способ – склеить биты x и y
            std::size_t h1 = std::hash<double>{}(p.getX());
            std::size_t h2 = std::hash<double>{}(p.getY());
            return h1 ^ (h2 << 1);
        }
    };
}


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_POINT_2_H