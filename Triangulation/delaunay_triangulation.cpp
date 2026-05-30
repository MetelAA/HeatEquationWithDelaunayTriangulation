#include <queue>
#include "delaunay_triangulation.h"

void triangulation::delaunay_triangulation::validateEdge(dcel::EdgeWrapper startEdge) {
    std::queue<dcel::EdgeWrapper> toValidate;
    toValidate.push(startEdge);


    while (!toValidate.empty()) {
        dcel::EdgeWrapper e = toValidate.front();
        toValidate.pop();

        if (!e.hasValidTwin()) continue; //если мы попали на одни из ребер соединяющих 3 начальные вершины между собой, то у него нет твинов и оно не разу не измениться
        if (e.getSourceVertex().is_infinite() && e.getNextEdge().getSourceVertex().is_infinite()) continue;
        if (isEdgeInvalid(e)) {
            size_t node0_index = e.getFace().getDelaunayNodeIndex();
            size_t node1_index = e.getTwinEdge().getFace().getDelaunayNodeIndex();
            dcel::EdgeWrapper newE = dcel.flip_edge(e);

            std::vector<size_t> old_leafs_indexes = {node0_index, node1_index};
            std::vector<dcel::FaceWrapper> new_faces = {newE.getFace(), newE.getTwinEdge().getFace()};
            std::vector<size_t> new_leafs_indexes = tree.insert(new_faces,
                                                                old_leafs_indexes); //у меня реализовано что сразу для двух делается
            //см. комменатрии под tree.insert чтобы понять что тут происходит и почему именно так((((
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


bool triangulation::delaunay_triangulation::isEdgeInvalid(dcel::EdgeWrapper e) {
    if (e.getSourceVertex().is_infinite() || e.getNextEdge().getSourceVertex().is_infinite()) {
        //наше ребро инцидентно одной бесокнечной вершине, двум уже не может т.к. отсеили до вызова этого метода


        dcel::EdgeWrapper e_twin = e.getTwinEdge();

        // ищем конечное ребро в twin-грани
        dcel::EdgeWrapper finite_edge = e_twin;
        bool found_finite = false;
        {
            dcel::EdgeWrapper iter = e_twin;
            for (int attempt = 0; attempt < 3; ++attempt) {
                if (!iter.getSourceVertex().is_infinite() && !iter.getNextEdge().getSourceVertex().is_infinite()) {
                    finite_edge = iter;
                    found_finite = true;
                    break;
                }
                iter = iter.getNextEdge();
            }
        }
        if (!found_finite) return false; //если не нашли конечное, то гг, бесконечные мы не трогаем

        dcel::VertexWrapper D = e.getPrevEdge().getSourceVertex();
        if (D.is_infinite()) return false; // если D бесконечна — флип невозможен и не нужен, вроде как не очень возможна такая ситуация, но лишнем не будет

        point_2 finiteSource = finite_edge.getSourceVertex().getGeometry();
        point_2 finiteTarget = finite_edge.getNextEdge().getSourceVertex().getGeometry();
        point_2 pd = D.getGeometry(); //противолежащая

        return vector_s::vector_orientation(finiteSource, finiteTarget, pd) == vector_s::orientation::left;
    }

    // Оба конца ребра e — конечные
    // Проверяем вершины twin-грани и противолежащую вершину на бесконечность.
    dcel::VertexWrapper e_target = e.getTwinEdge().getSourceVertex();
    dcel::VertexWrapper e_source = e.getSourceVertex();
    dcel::VertexWrapper twin_opposite = e.getTwinEdge().getPrevEdge().getSourceVertex();
    dcel::VertexWrapper e_opposite   = e.getPrevEdge().getSourceVertex();

    if (twin_opposite.is_infinite() || e_opposite.is_infinite()) //возможно не верно
        return false;

    return vector_s::in_circle(
            e_target.getGeometry(),
            e_source.getGeometry(),
            twin_opposite.getGeometry(),
            e_opposite.getGeometry());
}


void triangulation::delaunay_triangulation::triangulate() {
    for (int i = 1; i < points.size(); ++i) { //с 1 тк 0 уже в конструкторе пользовали!
        const point_2 &p = points[i];
        delaunay_tree::point_location location = tree.locate(p);
        //вытаскиваем индексы нод (Leaf) в которые попала точка перед сплитом
        std::vector<size_t> old_leafs_indexes = {location.face.getDelaunayNodeIndex()};
        if (location.edge.has_value()) {
            size_t neighbour_idx = location.edge->getTwinEdge().getFace().getDelaunayNodeIndex();
            old_leafs_indexes.push_back(neighbour_idx);
        }
        dcel::VertexWrapper newVertex = dcel.split_face(location.face, location.edge, p);

        std::vector<dcel::FaceWrapper> new_faces = dcel.get_incident_faces(newVertex);
        std::vector<size_t> new_leafs_indexes = tree.insert(new_faces, old_leafs_indexes);
        //см. комменатрии под tree.insert чтобы понять что тут происходит и почему именно так((((
        if (new_leafs_indexes.size() != new_faces.size())
            throw std::runtime_error("Undefined behavior, size of new_leaf_indexes must be equal to new_faces size");

        for (int j = 0; j < new_faces.size(); ++j) {
            dcel.update_face_node_index(new_faces[j],
                                        new_leafs_indexes[j]); //во тут уже говнокод, но я не буду проверять а потом искать ошибку в пробрассывании корректной ссылки на face, тем более тут не ссылки а копии везде
        }

        for (dcel::EdgeWrapper e : dcel.get_outgoing_edges(newVertex)) {
            validateEdge(e);
        }
    }
}

triangulation::delaunay_triangulation::delaunay_triangulation(std::vector<point_2> &pts) {
    auto highestPoint = pts.begin();
    for (auto it = pts.begin(); it != pts.end(); ++it) {
        if (it->getY() > highestPoint->getY() ||
            (it->getY() == highestPoint->getY() && it->getX() > highestPoint->getX()))
            highestPoint = it;
    }
    std::iter_swap(pts.begin(), highestPoint);

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (size_t i = 1; i < pts.size(); ++i) {
        size_t j = i + static_cast<size_t>(std::rand()) % (pts.size() - i);
        std::swap(pts[i], pts[j]);
    }
    points = std::move(pts);

    dcel::FaceWrapper root_face = dcel.init_dcel_with_big_inf_triangle(points[0]);

    tree = delaunay_tree(root_face, points.size());

    triangulate();
}

dto::TriangulationResult triangulation::delaunay_triangulation::getTriangulationResult(
        const std::unordered_map<point_2, point_2> &boundary_vertexes_map) {
    return dcel.getTriangulationWithCorrectBoundary(boundary_vertexes_map);
}
