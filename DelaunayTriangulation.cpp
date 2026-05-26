#include "DelaunayTriangulation.h"

void DelaunayTriangulation::validateEdge(DCEL::EdgeWrapper e) {
    if (e.getSourceVertex().is_infinite() || e.getNextEdge().getSourceVertex().is_infinite()) return;
    if (isEdgeInvalid(e)){
        size_t node0_index = e.getFace().getDelaunayNodeIndex();
        size_t node1_index = e.getTwinEdge().getFace().getDelaunayNodeIndex();
        DCEL::EdgeWrapper newE = dcel.flip_edge(e);

        std::vector<size_t> old_leafs_indexes = {node0_index, node1_index};
        std::vector<DCEL::FaceWrapper> newFaces = {newE.getFace(), newE.getTwinEdge().getFace()};
        tree.insert(newFaces, old_leafs_indexes); //у меня реализовано что сразу для двух делается
        validateEdge(newE.getNextEdge());
        validateEdge(newE.getTwinEdge().getPrevEdge());
    }
}

bool DelaunayTriangulation::isEdgeInvalid(DCEL::EdgeWrapper e) {
    if (e.getSourceVertex().is_infinite() || e.getNextEdge().getSourceVertex().is_infinite()){
        //наше ребро инцидентно одной бесокнечной вершине, двум уже не может т.к. вызывается после split_face, т.е. уже минимум 3 face

        DCEL::EdgeWrapper e_twin = e.getTwinEdge();

        // найдём конечное ребро (через тернарник тк нет пустого конструктора)
        DCEL::EdgeWrapper finite_edge = e_twin.getSourceVertex().is_infinite() ? e_twin.getNextEdge() : e_twin.getPrevEdge();
        //два варианта:
        // e_twin идёт из бесконечности в конечную вершину => следующим в CCW-обходе будет конечное ребро
        // e_twin идёт из конечной вершины в бесконечность => предыдущее ребро в CCW-обходе – конечное


        // Противолежащая вершина относительно ребра |e| в его собственной грани
        DCEL::VertexWrapper opposite = e.getPrevEdge().getSourceVertex();

        // sorce и target конечного ребра
        Point_2 fin_source = finite_edge.getSourceVertex().getGeometry();
        Point_2 fin_target = finite_edge.getNextEdge().getSourceVertex().getGeometry();
        Point_2 p_opposite = opposite.getGeometry();

        auto orient = vector_s::vector_orientation(fin_source, fin_target, p_opposite);
        return orient == vector_s::orientation::left;
    }

    return vector_s::in_circle(e.getTwinEdge().getSourceVertex().getGeometry(),
            e.getTwinEdge().getNextEdge().getSourceVertex().getGeometry(),
            e.getTwinEdge().getPrevEdge().getSourceVertex().getGeometry(),
            e.getPrevEdge().getSourceVertex().getGeometry());
}

void DelaunayTriangulation::triangulate() {
    for (int i = 1; i < points.size(); ++i) { //с 1 тк 0 уже в конструкторе пользовали!
        const Point_2& p = points[i];
        DelaunayTree::point_location location = tree.locate(p);
        //вытаскиваем индексы нод (Leaf) в которые попала точка перед сплитом
        std::vector<size_t> old_leafs_indexes = {location.face.getDelaunayNodeIndex()};
        if (location.edge.has_value()) {
            size_t neighbour_idx = location.edge->getTwinEdge().getFace().getDelaunayNodeIndex();
            old_leafs_indexes.push_back(neighbour_idx);
        }
        DCEL::VertexWrapper newVertex = dcel.split_face(location.face, location.edge, p);

        std::vector<DCEL::FaceWrapper> new_faces = dcel.get_incident_faces(newVertex);
        std::vector<size_t> new_leafs_indexes = tree.insert(new_faces, old_leafs_indexes);
        //см. комменатрии под tree.insert чтобы понять что тут происходит и почему именно так((((
        if (new_leafs_indexes.size() != new_faces.size())
            throw std::runtime_error("Undefined behavior, size of new_leaf_indexes must be equal to new_faces size");

        for (int j = 0; j < new_faces.size(); ++j) {
            dcel.update_face_node_index(new_faces[j], new_leafs_indexes[j]); //во тут уже говнокод, но я не буду проверять а потом искать ошибку в пробрассывании корректной ссылки на face, тем более тут не ссылки а копии везде
        }
        for(DCEL::EdgeWrapper e : dcel.get_outgoing_edges(newVertex)){
            validateEdge(e);
        }
    }
}

DelaunayTriangulation::DelaunayTriangulation(std::vector<Point_2> &pts) {
    auto highestPoint = pts.begin();
    for (auto it = pts.begin(); it != pts.end(); ++it) {
        if (it->getY() > highestPoint->getY() || (it->getY() == highestPoint->getY() && it->getX() > highestPoint->getX()))
            highestPoint = it;
    }
    std::iter_swap(pts.begin(), highestPoint);

    // случайгая перестановка
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (size_t i = 1; i < pts.size(); ++i) {
        size_t j = i + std::rand() % (pts.size() - i);
        std::swap(pts[i], pts[j]);
    }
    points = std::move(pts);

    // цнициализириуем DCEL начальным большим треугольником
    DCEL::FaceWrapper root_face = dcel.init_dcel_with_big_inf_triangle(points[0]);

    tree = DelaunayTree(root_face, points.size());
}

