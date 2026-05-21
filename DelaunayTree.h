//
// Created by Artem on 21.05.2026.
//

#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H

#include <memory>
#include "DCEL.h"
#include "vector_stuff.h"

class DelaunayTree {
public:
    struct location_data;

    DelaunayTree(size_t pointsCount) {
        this->nodes.reserve(pointsCount + 1); // важно! - чтобы не было висячих указателей в векторе children в Internal
    }

    void insert(size_t node_index, DCEL<>::FaceWrapper face); // я так и не понял, почему мы вставляем не точку а что мы тут вообще вставляем????????

    void insert(size_t first_node_index, size_t second_node_index, DCEL<>::FaceWrapper face);

    location_data locate(Point_2 &p);

    struct location_data {
        DCEL<>::FaceWrapper face;
        DCEL<>::EdgeWrapper edge;
    };

protected:
    class Node {
    public:
        Node(const DCEL<>::VertexWrapper &v1, const DCEL<>::VertexWrapper &v2, const DCEL<>::VertexWrapper &v3,
             size_t index_in_nodes_vector) :
                v1(v1), v2(v2), v3(v3), index_in_nodes_vector(index_in_nodes_vector) {
            //сделаем проверку на правильность ориентации ребер (обход против часовой)
            if (vector_s::vector_orientation(v1.getGeometry(), v2.getGeometry(), v3.getGeometry()) !=
                vector_s::orientation::left)
                throw "orientation of triangle bypass must be left!";
        }

        const DCEL<>::VertexWrapper &getV1() const { return v1; }

        const DCEL<>::VertexWrapper &getV2() const { return v2; }

        const DCEL<>::VertexWrapper &getV3() const { return v3; }

        virtual Node const *locate(Point_2 &p,
                                   std::unique_ptr<location_data>& locationData) const = 0; // p - исходная точка откуда запускается поиск, не забыть создать DTO struct чтобы передать отсюда важные данные, а не оставить функцию void


    private:
        DCEL<>::VertexWrapper v1, v2, v3; //обход треугольника против часовой
        size_t index_in_nodes_vector;
    };

    class Leaf : public Node {
    public:
        Leaf(const DCEL<>::VertexWrapper &v1, const DCEL<>::VertexWrapper &v2, const DCEL<>::VertexWrapper &v3,
             size_t indexInNodesVector, const DCEL<>::FaceWrapper &currentFace) : Node(v1, v2, v3, indexInNodesVector),
                                                                                  current_face(currentFace) {}

        Node const *locate(Point_2 &p, std::unique_ptr<location_data>& locationData) const override {
            locationData = std::make_unique<location_data>(location_data{current_face, current_face.getEdgeInWrapper()});
            return nullptr;
        }

    private:
        DCEL<>::FaceWrapper current_face; //грань к которой относиться эта нода
    };

    class Internal : public Node {
    public:
        Internal(const DCEL<>::VertexWrapper &v1, const DCEL<>::VertexWrapper &v2, const DCEL<>::VertexWrapper &v3,
                 size_t indexInNodesVector, std::vector<Node *> &children) : Node(v1, v2, v3, indexInNodesVector),
                                                                             children(children) {}
        Internal(const Node& node,
                 size_t indexInNodesVector, std::vector<Node *> &children) : Node(node.getV1(), node.getV2(), node.getV3(), indexInNodesVector),
                                                                             children(children) {}
        Node const *locate(Point_2 &p, std::unique_ptr<location_data>& locationData) const override {
            for (Node *child: children) {
                if (vector_s::vector_orientation(child->getV1().getGeometry(), child->getV2().getGeometry(), p) != vector_s::orientation::right &&
                        vector_s::vector_orientation(child->getV2().getGeometry(), child->getV3().getGeometry(), p) != vector_s::orientation::right &&
                        vector_s::vector_orientation(child->getV3().getGeometry(), child->getV1().getGeometry(), p) != vector_s::orientation::right
                ) {
                    return child;
                }
            }
            throw "something unexpected, we in internal node which not contains any children with out point inside";
        }

//        static double calculate_point_in_circle_determinant(const Point_2& p, const Point_2 q, const Point_2 r, const Point_2 s);
    private:
        std::vector<Node *> children;
    };

    std::vector<std::unique_ptr<Node>> nodes;
};



#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DELAUNAYTREE_H
