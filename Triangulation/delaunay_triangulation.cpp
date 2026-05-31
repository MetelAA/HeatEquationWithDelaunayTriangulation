#include <queue>
#include "delaunay_triangulation.h"
void triangulation::delaunay_triangulation::validateEdge(dcel::EdgeWrapper startEdge) {
    std::queue<dcel::EdgeWrapper> toValidate;
    toValidate.push(startEdge);

    while (!toValidate.empty()) {
        dcel::EdgeWrapper e = toValidate.front();
        toValidate.pop();
        if (isEdgeInvalid(e)) {
            size_t node0_index = e.getFace().getDelaunayNodeIndex();
            size_t node1_index = e.getTwinEdge().getFace().getDelaunayNodeIndex();
            dcel::EdgeWrapper newE = dcel.flip_edge(e);
            std::vector<dcel::FaceWrapper> new_faces = {newE.getFace(), newE.getTwinEdge().getFace()};
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


bool triangulation::delaunay_triangulation::isEdgeInvalid(dcel::EdgeWrapper e) { //если twin - существует, то существует соседняя грань полностью
    dcel::VertexWrapper e_target = e.getTwinEdge().getSourceVertex();
    dcel::VertexWrapper e_source = e.getSourceVertex();
    dcel::VertexWrapper e_opposite   = e.getPrevEdge().getSourceVertex();
    dcel::VertexWrapper twin_opposite = e.getTwinEdge().getPrevEdge().getSourceVertex();

    if (e_source.is_infinite() || e_target.is_infinite() || e_opposite.is_infinite()) return false;
    //если ребро e принадлжеит бесконечной грани то его не флипаем, в ином

    //проверяем что twin грань лежит в небесконечном треугольнике
    if (twin_opposite.is_infinite()) return false;

    return vector_s::in_circle(e_target.getGeometry(), e_source.getGeometry(),
                           twin_opposite.getGeometry(), e_opposite.getGeometry());
}


void triangulation::delaunay_triangulation::triangulate() {
    for (int i = 0; i < points.size(); ++i) {
        const point_2 &p = points[i];
        delaunay_tree::point_location location = tree.locate(p);
        std::vector<size_t> old_leafs_indexes = {location.face.getDelaunayNodeIndex()};
        if (location.edge.has_value()) {
            size_t neighbour_idx = location.edge->getTwinEdge().getFace().getDelaunayNodeIndex();
            old_leafs_indexes.push_back(neighbour_idx);
        }
        dcel::VertexWrapper newVertex = dcel.split_face(location.face, location.edge, p);
        std::vector<dcel::FaceWrapper> new_faces = dcel.get_incident_faces(newVertex);
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

        for (dcel::EdgeWrapper e : dcel.get_outgoing_edges(newVertex))
            validateEdge(e);
    }
}

triangulation::delaunay_triangulation::delaunay_triangulation(std::vector<point_2> &pts) {
    //теперь нам насрать какая точка самая верхняя и самая нижняя, тк есть супер треугольник

    //добавить мешалку (потом)

    points = std::move(pts);

    std::vector<dcel::FaceWrapper> root_faces = dcel.init_dcel_with_big_inf_triangle(points.size());

    tree = delaunay_tree(root_faces, points.size());

    triangulate();
}

dto::TriangulationResult triangulation::delaunay_triangulation::getTriangulationResult(
        const std::unordered_map<point_2, point_2> &boundary_vertexes_map) {
    return dcel.getTriangulationWithCorrectBoundary(boundary_vertexes_map);
}