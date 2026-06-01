#include <queue>
#include "DelaunayTriangulation.h"


bool Triangulation::DelaunayTriangulation::isEdgeInvalid(DCEL::EdgeWrapper e) { //если twin - существует, то существует соседняя грань полностью
    DCEL::VertexWrapper e_target = e.getTwinEdge().getSourceVertex();
    DCEL::VertexWrapper e_source = e.getSourceVertex();
    DCEL::VertexWrapper e_opposite   = e.getPrevEdge().getSourceVertex();
    DCEL::VertexWrapper twin_opposite = e.getTwinEdge().getPrevEdge().getSourceVertex();

    if (e_source.is_infinite() || e_target.is_infinite() || e_opposite.is_infinite()) return false;
    //если ребро e принадлжеит бесконечной грани то его не флипаем, в ином

    //проверяем что twin грань лежит в небесконечном треугольнике
    if (twin_opposite.is_infinite()) return false;

    return vector_s::in_circle(e_target.getGeometry(), e_source.getGeometry(),
                           twin_opposite.getGeometry(), e_opposite.getGeometry());
}


void Triangulation::DelaunayTriangulation::validateEdge(DCEL::EdgeWrapper startEdge) {
    std::queue<DCEL::EdgeWrapper> toValidate;
    toValidate.push(startEdge);

    while (!toValidate.empty()) {
        DCEL::EdgeWrapper e = toValidate.front();
        toValidate.pop();
        if (isEdgeInvalid(e)) {
            size_t node0_index = e.getFace().getDelaunayNodeIndex();
            size_t node1_index = e.getTwinEdge().getFace().getDelaunayNodeIndex();
            DCEL::EdgeWrapper newE = dcel.flip_edge(e);
            std::vector<DCEL::FaceWrapper> new_faces = {newE.getFace(), newE.getTwinEdge().getFace()};
            std::vector<size_t> new_leafs_indexes = tree.insertWhenFlip(new_faces, node0_index, node1_index);
            for (int i = 0; i < new_leafs_indexes.size(); ++i) {
                dcel.update_face_node_index(new_faces[i], new_leafs_indexes[i]);
            }

            toValidate.push(newE.getNextEdge());
            toValidate.push(newE.getPrevEdge());
            toValidate.push(newE.getTwinEdge().getNextEdge());
            toValidate.push(newE.getTwinEdge().getPrevEdge());
        }
    }
}


void Triangulation::DelaunayTriangulation::triangulate() {
    for (int i = 0; i < points.size(); ++i) {
        const Point_2 &p = points[i];
        DelaunayTree::point_location location = tree.locate(p);
        std::vector<size_t> old_leafs_indexes = {location.face.getDelaunayNodeIndex()};
        if (location.edge.has_value()) {
            size_t neighbour_idx = location.edge->getTwinEdge().getFace().getDelaunayNodeIndex();
            old_leafs_indexes.push_back(neighbour_idx);
        }
        DCEL::VertexWrapper newVertex = dcel.split_face(location.face, location.edge, p);
        std::vector<DCEL::FaceWrapper> new_faces = dcel.get_incident_faces(newVertex);
        std::vector<size_t> new_leafs_indexes;
        if (!location.edge.has_value()) {
            if (old_leafs_indexes.size() != 1)
                throw std::runtime_error("Undefined behavior must be only one parent when new point in one face");
            new_leafs_indexes = tree.insertWhenOneFace(new_faces, old_leafs_indexes[0]);
        }else {
            if (old_leafs_indexes.size() != 2)
                throw std::runtime_error("Undefined behavior must be two parents when new point on a edge in face");
            new_leafs_indexes = tree.insertWhenEdge(new_faces, old_leafs_indexes[0], old_leafs_indexes[1]);
        }
        if (new_leafs_indexes.size() != new_faces.size())
            throw std::runtime_error("Undefined behavior, size of new_leaf_indexes must be equal to new_faces size");

        for (int j = 0; j < new_faces.size(); ++j) {
            dcel.update_face_node_index(new_faces[j], new_leafs_indexes[j]);
        }

        for (DCEL::EdgeWrapper e : dcel.get_outgoing_edges(newVertex))
            validateEdge(e.getNextEdge());
    }
}

// (конструктор и getTriangulationResult остаются без изменений)
Triangulation::DelaunayTriangulation::DelaunayTriangulation(std::vector<Point_2> &pts) {
    points = std::move(pts);
    std::vector<DCEL::FaceWrapper> root_faces = dcel.init_dcel_with_big_inf_triangle(points.size());
    tree = DelaunayTree(root_faces, points.size());
    triangulate();
}

DTO::TriangulationResult Triangulation::DelaunayTriangulation::getTriangulationResult() {
    return dcel.getTriangulationWithCorrectBoundary();
}