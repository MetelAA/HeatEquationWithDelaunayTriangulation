#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TREE_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TREE_H

#include <stdexcept>
#include <memory>
#include <cmath>
#include <iostream>
#include <utility>
#include "dcel.h"
#include "vector_stuff.h"

class delaunay_tree {
public:
    delaunay_tree() {} //пришлось создать инчае гемор, используется один раз и тут же перезабивается нормальным

    delaunay_tree(const std::vector<dcel::FaceWrapper>& rootFaces, size_t points_count) {
        size_t log2n = static_cast<size_t>(std::ceil(std::log2(points_count)));
        nodes_.reserve(10 * points_count * log2n + 64);

        roots_count = 0;
        for (dcel::FaceWrapper face : rootFaces) {
            dcel::EdgeWrapper e = face.getEdge();
            dcel::VertexWrapper v0 = e.getSourceVertex();
            dcel::VertexWrapper v1 = e.getNextEdge().getSourceVertex();
            dcel::VertexWrapper v2 = e.getNextEdge().getNextEdge().getSourceVertex();

            //будем выполнять соглашение, что если root грань содержит точку на бесконечности, то она находиться на месте v0, позволит не городить циклы с поиском конечной грани в locate для поиска конечного ребра
            //я не уверен что после таких свапов не нужно перестраивать треугольник, но учитывая что p_inf - находиться просто где-то, и конечное ребро - всегда твин какого-то ребра из конечного супертреугольника (face3)
            //то как будто без разницы, так что пока оставим). - НЕТ ТОЧНО сломается, проверил на бумаге, так что кидаем ошибку если что-то не так
            if (v1.is_infinite() || v2.is_infinite())
                throw std::runtime_error("error!!! if triangle has a infinite vertex it must be on a v0!!!");

            nodes_.emplace_back(std::make_unique<Leaf>(v0, v1, v2, this, face));
            roots_count++;
        }
    }

    struct point_location { //куда попала точка которую мы пытаемся вставить, всегда FaceWrapper (грань) и если попали то EdgeWrapper (ребро на которое попали)
        dcel::FaceWrapper face;
        std::optional<dcel::EdgeWrapper> edge;

        point_location(const dcel::FaceWrapper &face, const std::optional<dcel::EdgeWrapper> &edge) : face(face),
                                                                                                      edge(edge) {}
    };

    point_location locate(const point_2 &point) {
        std::optional<point_location> loc;
        for (int i = roots_count-1; i >= 0; i--) {
            size_t cur_idx = i;
            while (true) {
                auto next = nodes_.at(cur_idx)->locate(point, loc);
                if (!next.has_value()) break;
                cur_idx = *next;
            }
            if (loc.has_value())
                return *loc;
        }

        if (!loc.has_value()) throw std::runtime_error("Undefined behavior, point location failed!!!!");
        return *loc;
    }


    //сделать insert как в презе не получиться, тк мы имеем дело с указателями, мы не имеет в Node флага является ли она листом (а заводить такой как будто плотный навал кринжа)
    //поэтому когда мы добавляем очереднего ребёнка мы не понимаем, его родитель уже Internal или ещё Leaf. Плюсом даже если добавить флаг, будет постоянный апкаст, а апкаст это харам!

    //Таким образом, у нас будет 3 метода insert - первый для случая попадания внутрь грани, на вход 3 ребёнка+индекс родителя, на выход 3 индекса новых Leaf для детей (в той же последовательности как заходили дети)
    // второй - для попадания на грань, 4 ребёнка, два индекса родителя, придётся посортировать какой ребёнок от какого родителя, на выход тоже самое (самый кринжовый метод из троицы)
    // третий - для flip edge на вход - 2 ребёнка, два индекса родителя, на выход также
    std::vector<size_t> insertWhenOneFace(const std::vector<dcel::FaceWrapper> &newFaces, size_t old_leaf_index);
    std::vector<size_t> insertWhenEdge(const std::vector<dcel::FaceWrapper> &newFaces, size_t old_leaf_index1, size_t old_leaf_index2);
    std::vector<size_t> insertWhenFlip(const std::vector<dcel::FaceWrapper> &newFaces, size_t old_leaf_index1, size_t old_leaf_index2);



private:
    std::vector<size_t> insertNewLeafs(const std::vector<dcel::FaceWrapper>& newFaces); //возвращает индексы вставленных leafs в том же порядке как поступили faces

    class Node {
    public:
        Node(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree* tree)
                : v1(v1), v2(v2), v3(v3), tree(tree) {
            //сделаем проверку на правильность ориентации ребер (обход против часовой)
            if (!v1.is_infinite() && !v2.is_infinite() && !v3.is_infinite()) {
                if (vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), v3.getGeometry()) !=
                    vector_s::orientation::left) {
                    std::cout << "v1: x |" << v1.getGeometry().getX() << "|, y |" << v1.getGeometry().getY() << "|; " << "v2: x |" << v2.getGeometry().getX() << "|, y |" << v2.getGeometry().getY() <<"|; "
        << "v3: x |" << v3.getGeometry().getX() << "|, y |" << v3.getGeometry().getY() << "|\n";
                    throw std::runtime_error("orientation of triangle bypass must be left!");
                }

            }
        }

        virtual ~Node() = default;

        virtual std::optional<size_t> locate(point_2 const &p, std::optional<point_location> &location) const = 0;

        const dcel::VertexWrapper &getV1() const { return v1; }

        const dcel::VertexWrapper &getV2() const { return v2; }

        const dcel::VertexWrapper &getV3() const { return v3; }

    protected:
        dcel::VertexWrapper v1, v2, v3; //врапперы вершин образующих грань
        delaunay_tree *tree;
    };

    class Internal : public Node {
    public:
        Internal(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree *tree,
                 std::vector<size_t> children_indexes) : Node(v1, v2, v3, tree), children_indexes(std::move(children_indexes)) {}

        virtual std::optional<size_t> locate(const point_2 &p, std::optional<point_location> &location) const override {
            for (const size_t child_index : children_indexes) {
                auto child = tree->nodes_[child_index].get();
                if (point_in_node(child->getV1(), child->getV2(), child->getV3(), p)) {
                    return child_index;
                }
            }
        }


    private:
        std::vector<size_t> children_indexes;


        static bool point_in_node(const dcel::VertexWrapper &v0, const dcel::VertexWrapper &v1,
                                  const dcel::VertexWrapper &v2, const point_2 &p) {
            if (v0.is_infinite()) {
                //логика с полуплоскостями, учитывая что v0 - inf, значит конечное ребро образуют v1, v2
                return vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), p) != vector_s::orientation::right;
            }
            auto o1 = vector_s::vector_orientation(v0.getGeometry(), v1.getGeometry(), p);
            auto o2 = vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), p);
            auto o3 = vector_s::vector_orientation(v2.getGeometry(), v0.getGeometry(), p);
            return o1 != vector_s::orientation::right &&
                   o2 != vector_s::orientation::right &&
                   o3 != vector_s::orientation::right;
        }
    };

    class Leaf : public Node {
    public:
        Leaf(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree *tree,
             const dcel::FaceWrapper &face) : Node(v1, v2, v3, tree), face(face) {}

        virtual std::optional<size_t> locate(const point_2 &p, std::optional<point_location> &location) const override {
            if (v1.is_infinite()) { //Если грань содержит хотя бы одну бесконечную вершину, точка не может лежать на ребре с такой вершиной
                location.emplace(face, std::nullopt);
                return std::nullopt;
            }

            point_2 p1 = this->v1.getGeometry();
            point_2 p2 = this->v2.getGeometry();
            point_2 p3 = this->v3.getGeometry();
            if (vector_s::vector_orientation(p1, p2, p) ==
                vector_s::orientation::collinear) { //ориетация против часовой (буду писать это везде иначе сам запутаюсь, хз почему в МФТИ лекция дана правая?)
                location.emplace(face, findEdge(v1, v2));
                } else if (vector_s::vector_orientation(p2, p3, p) == vector_s::orientation::collinear) {
                    location.emplace(face, findEdge(v2, v3));
                } else if (vector_s::vector_orientation(p3, p1, p) == vector_s::orientation::collinear) {
                    location.emplace(face, findEdge(v3, v1));
                } else {
                    location.emplace(face, std::nullopt);
                }

            return std::nullopt; //завершаем цикл в locate!
        }

    private:
        dcel::FaceWrapper face; //текущая существующая в dcel грань
        dcel::EdgeWrapper findEdge(const dcel::VertexWrapper &a, const dcel::VertexWrapper &b) const {
            dcel::EdgeWrapper startEdgeInFace = face.getEdge();
            dcel::EdgeWrapper currentEdge = startEdgeInFace;
            do {
                if (currentEdge.getSourceVertex() == a && currentEdge.getNextEdge().getSourceVertex() == b)
                    return currentEdge;
                currentEdge = currentEdge.getNextEdge();
            } while (!(currentEdge == startEdgeInFace));
            throw std::runtime_error("collinear edge not found");
        }

    };

    std::vector<std::unique_ptr<Node>> nodes_;
    int roots_count;
};

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TREE_H
