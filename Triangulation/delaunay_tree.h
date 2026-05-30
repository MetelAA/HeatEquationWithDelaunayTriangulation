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

    delaunay_tree(dcel::FaceWrapper& root_face, size_t points_count) {
        size_t log2n = static_cast<size_t>(std::ceil(std::log2(points_count)));

        nodes_.reserve(10 * points_count * log2n + 64);//зарезервировать места под все точки что будут, иначе указатели в Internal поедут

        dcel::EdgeWrapper e = root_face.getEdge();
        dcel::VertexWrapper v0 = e.getSourceVertex();
        dcel::VertexWrapper v1 = e.getNextEdge().getSourceVertex();
        dcel::VertexWrapper v2 = e.getNextEdge().getNextEdge().getSourceVertex();
        nodes_.emplace_back(std::make_unique<Leaf>(v0, v1, v2, this, root_face));
    }

    struct point_location { //куда попала точка которую мы пытаемся вставить, всегда FaceWrapper (грань) и если попали то EdgeWrapper (ребро на которое попали)
        dcel::FaceWrapper face;
        std::optional<dcel::EdgeWrapper> edge;

        point_location(const dcel::FaceWrapper &face, const std::optional<dcel::EdgeWrapper> &edge) : face(face),
                                                                                                      edge(edge) {}
    };

    point_location locate(const point_2 &point) {
        std::optional<point_location> loc;
        size_t cur_idx = 0;
        while (true) {
            auto next = nodes_.at(cur_idx)->locate(point, loc);
            if (!next.has_value()) break;
            cur_idx = *next;
        }
        if (!loc.has_value()) throw std::runtime_error("Undefined behavior, point location failed!!!!");
        return *loc;
    }

    std::vector<size_t> insert(std::vector<dcel::FaceWrapper> &newFaces,
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
        Node(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree* tree)
                : v1(v1), v2(v2), v3(v3), tree(tree) {
            //сделаем проверку на правильность ориентации ребер (обход против часовой)
            if (!v1.is_infinite() && !v2.is_infinite() && !v3.is_infinite()) {
                if (vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), v3.getGeometry()) !=
                    vector_s::orientation::left) {
                    std::cout << "v1: x |" << v1.getGeometry().getX() << "|, y |" << v1.getGeometry().getY() << "|; " << "v2: x |" << v2.getGeometry().getX() << "|, y |" << v2.getGeometry().getY() <<"|; "
        << "v3: x |" << v3.getGeometry().getX() << "|, y |" << v3.getGeometry().getY() << "|\n";;
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
        delaunay_tree* tree; //указатель на дерево к которому принадлежим
    };

    class Internal : public Node {
    public:
        Internal(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree* tree,
                 std::vector<size_t> children_indexes) : Node(v1, v2, v3, tree), children_indexes(std::move(children_indexes)) {}

        virtual std::optional<size_t> locate(const point_2 &p, std::optional<point_location> &location) const override {
            //возникает проблема что в триангуляции всегда продолжает присутствовать грань с двумя бесконечными вершинами, а для такого случая
            //мы не умеем нормально определять попадание в грань, городить что-то на уровне одной бесокнечной вершины супер-треугольников (как в cg_lib) я не буду, задавать
            //какие-то координаты для отрицательных вершин тоже считаю не правильным, оно точно где-нибудь поломается и я это никогда не отдебажу, поэтому,
            //вместо простого перебора всех детей подряд, мы для каждого количества точек на бесконечности (от 0 до 2) перебираем детей и пропускаем СНАЧАЛА тех, у которых количество бесконечных вершин не равно 2.
            //Ну типо сначала 0 бесконечных, потом 1 бесконечная, потом 2. В 99 проц случаев закончиться на первой итерации внешнего цикла!
            for (int max_inf = 0; max_inf <= 2; ++max_inf) {
                size_t child_index;
                for (int i = 0; i < children_indexes.size(); ++i) {
                    child_index = children_indexes[i];
                    auto child = tree->nodes_[child_index].get();
                    int inf_count = child->getV1().is_infinite() +
                                    child->getV2().is_infinite() +
                                    child->getV3().is_infinite();
                    if (inf_count != max_inf) continue;

                    if (point_in_node(child->getV1(), child->getV2(), child->getV3(), p)) {
                        return child_index;
                    }
                }
            }

            throw std::runtime_error(
                    "Undefined behavior, we shouldn't went to the Internal Node with no children_indexes with our point inside!");
        }

    private:
        std::vector<size_t> children_indexes;


        static bool point_in_node(const dcel::VertexWrapper &a, const dcel::VertexWrapper &b,
                                  const dcel::VertexWrapper &c, const point_2 &p) {
            bool inf1 = a.is_infinite();
            bool inf2 = b.is_infinite();
            bool inf3 = c.is_infinite();
            int inf_count = inf1 + inf2 + inf3;

            if (inf_count == 2) {
                // попали в большой треугольник, а значит точка всегда внутри
                return true;
            } else if (inf_count == 1) {
                // Одна бесконечная вершина: внутренность – полуплоскость слева от конечного ребра
                const dcel::VertexWrapper *x = nullptr, *y = nullptr;
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
        Leaf(const dcel::VertexWrapper &v1, const dcel::VertexWrapper &v2, const dcel::VertexWrapper &v3, delaunay_tree* tree,
             const dcel::FaceWrapper &face) : Node(v1, v2, v3, tree), face(face) {}

        virtual std::optional<size_t> locate(const point_2 &p, std::optional<point_location> &location) const override {

            if (v1.is_infinite() || v2.is_infinite() || v3.is_infinite()) { //Если грань содержит хотя бы одну бесконечную вершину, точка не может лежать на ребре с такой вершиной
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
};

#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAY_TREE_H
