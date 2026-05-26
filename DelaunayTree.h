#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H

#include <stdexcept>
#include <memory>
#include "DCEL.h"
#include "vector_stuff.h"
#include "DelaunayTriangulation.h"

class DelaunayTree {
public:
    DelaunayTree() {} //пришлось создать инчае гемор, используется один раз и тут же перезабивается нормальным

    DelaunayTree(DCEL::FaceWrapper& root_face, size_t points_count) {
        nodes_.reserve(points_count+33);//зарезервировать места под все точки что будут, иначе указатели в Internal поедут

        DCEL::EdgeWrapper e = root_face.getEdge();
        DCEL::VertexWrapper v0 = e.getSourceVertex();
        DCEL::VertexWrapper v1 = e.getNextEdge().getSourceVertex();
        DCEL::VertexWrapper v2 = e.getNextEdge().getNextEdge().getSourceVertex();
        nodes_.emplace_back(std::make_unique<Leaf>(v0, v1, v2, root_face));
    }

    struct point_location { //куда попала точка которую мы пытаемся вставить, всегда FaceWrapper (грань) и если попали то EdgeWrapper (ребро на которое попали)
        DCEL::FaceWrapper face;
        std::optional<DCEL::EdgeWrapper> edge;

        point_location(const DCEL::FaceWrapper &face, const std::optional<DCEL::EdgeWrapper> &edge) : face(face),
                                                                                                      edge(edge) {}
    };

    point_location locate(const Point_2 &point) {
        std::optional<point_location> loc;
        const Node *cur = nodes_.at(0).get();
        while (cur) {
            cur = cur->locate(point, loc);
        }
        if (!loc.has_value()) throw std::runtime_error("Undefined behavior, point location failed!!!!");
        return *loc;
    }

    std::vector<size_t> insert(std::vector<DCEL::FaceWrapper> &newFaces,
                               std::vector<size_t> &old_leafs_indexes); //если в old_leafs_indexes 1 индекс и в newFaces - 3 грани, значит попали внутрь грани,
    //если в old_leafs_indexes 2 индекса и в newFaces - 4 грани, значит попали на грань в этом случае нужно очень аккуратно работать с новыми гранями, отследить где чьи дети, т.к. incident_faces выдаст грани
    //в случайном порядке
    //кароч, вектор old_leafs_indexes заполняется так: если попали внутрь грани - face.getNodeIndex() если edge - не null, то из этого edga достаём обе face, т.е. не используем face которая в point_location (тк чтобы лишний раз не искать где что)
    //и вытаскиваем обе грани из этого ребра! ПРИ ЭТОМ ДЕЛАЕМ ЭТО ДО SPLIT_FACE т.к. есть шанс что после него на меcтах куда указывает face_index из враппера будет лежать уже новая face у которой это поле не заполнено вовсе
    //а возвращает вектор - индексы node к которым привязаны newFaces, т.е. после этого метода нужно в цикле вызывать dcel.update_face_node_index() c соответствующими facesWrappers и элементами результируюзего массива
    //newFaces[0] соотвествует [0] из вектора результата метода (нда....)
    //получается, что даже вершину сюда прокидывать не надо, мы её просто достаём из списка новых ребер


private:
    class Node {
    public:
        Node(const DCEL::VertexWrapper &v1, const DCEL::VertexWrapper &v2, const DCEL::VertexWrapper &v3)
                : v1(v1), v2(v2), v3(v3) {
            //сделаем проверку на правильность ориентации ребер (обход против часовой)
            if (!v1.is_infinite() && !v2.is_infinite() && !v3.is_infinite()) {
                if (vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), v3.getGeometry()) !=
                    vector_s::orientation::left)
                    throw std::runtime_error("orientation of triangle bypass must be left!");
            }
        }

        virtual Node const *locate(Point_2 const &p, std::optional<point_location> &location) const = 0;

        const DCEL::VertexWrapper &getV1() const { return v1; }

        const DCEL::VertexWrapper &getV2() const { return v2; }

        const DCEL::VertexWrapper &getV3() const { return v3; }

    protected:
        DCEL::VertexWrapper v1, v2, v3; //врапперы вершин образующих грань
    };

    class Internal : public Node {
    public:
        Internal(const DCEL::VertexWrapper &v1, const DCEL::VertexWrapper &v2, const DCEL::VertexWrapper &v3,
                 const std::vector<const Node *> &children) : Node(v1, v2, v3), children(children) {}

        Node const *locate(const Point_2 &p, std::optional<point_location> &location) const override {
            for (const Node *child : children) {
                if (point_in_node(child->getV1(), child->getV2(), child->getV3(), p)) {
                    return child;
                }
            }

            throw std::runtime_error(
                    "Undefined behavior, we shouldn't went to the Internal Node with no children with our point inside!");
        }

    private:
        std::vector<Node const *> children;


        static bool point_in_node(const DCEL::VertexWrapper &a, const DCEL::VertexWrapper &b,
                                  const DCEL::VertexWrapper &c, const Point_2 &p) {
            bool inf1 = a.is_infinite();
            bool inf2 = b.is_infinite();
            bool inf3 = c.is_infinite();
            int inf_count = inf1 + inf2 + inf3;

            if (inf_count == 2) {
                // попали в большой треугольник, а значит точка всегда внутри
                return true;
            } else if (inf_count == 1) {
                // Одна бесконечная вершина: внутренность – полуплоскость слева от конечного ребра
                const DCEL::VertexWrapper *x = nullptr, *y = nullptr;
                if (!inf1) {
                    x = &a;
                    y = (!inf2) ? &b : &c;
                } else if (!inf2) {
                    x = &b;
                    y = (!inf3) ? &c : &a;
                } else {
                    x = &c;
                    y = &a;
                }
                return vector_s::vector_orientation(x->getGeometry(), y->getGeometry(), p)
                       != vector_s::orientation::right;
            } else {
                // inf_count == 0 – обычный треугольник
                auto o1 = vector_s::vector_orientation(a.getGeometry(), b.getGeometry(), p);
                auto o2 = vector_s::vector_orientation(b.getGeometry(), c.getGeometry(), p);
                auto o3 = vector_s::vector_orientation(c.getGeometry(), a.getGeometry(), p);
                return o1 != vector_s::orientation::right &&
                       o2 != vector_s::orientation::right &&
                       o3 != vector_s::orientation::right;
            }
        }
    };

    class Leaf : public Node {
    public:
        Leaf(const DCEL::VertexWrapper &v1, const DCEL::VertexWrapper &v2, const DCEL::VertexWrapper &v3,
             const DCEL::FaceWrapper &face) : Node(v1, v2, v3), face(face) {}

        Node const *locate(const Point_2 &p, std::optional<point_location> &location) const override {

            if (v1.is_infinite() || v2.is_infinite() || v3.is_infinite()) { //Если грань содержит хотя бы одну бесконечную вершину, точка не может лежать на ребре с такой вершиной
                location.emplace(face, std::nullopt);
                return nullptr;
            }

            Point_2 p1 = this->v1.getGeometry();
            Point_2 p2 = this->v2.getGeometry();
            Point_2 p3 = this->v3.getGeometry();
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

            return nullptr; //завершаем цикл в locate!
        }

    private:
        DCEL::FaceWrapper face; //текущая существующая в DCEL грань

        DCEL::EdgeWrapper findEdge(const DCEL::VertexWrapper &a, const DCEL::VertexWrapper &b) const {
            DCEL::EdgeWrapper startEdgeInFace = face.getEdge();
            DCEL::EdgeWrapper currentEdge = startEdgeInFace;
            do {
                if (currentEdge.getSourceVertex() == a && currentEdge.getNextEdge().getSourceVertex() == b)
                    return currentEdge;
                currentEdge = currentEdge.getNextEdge();
            } while (!(currentEdge == startEdgeInFace));
            throw std::runtime_error("collinear edge not found");
        }

    };

    std::vector<std::unique_ptr<Node>> nodes_;
};

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H
