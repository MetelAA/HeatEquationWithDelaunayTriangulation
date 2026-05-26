#include "DelaunayTriangulation.h"

void Triangulation::DelaunayTriangulation::validateEdge(DCEL::EdgeWrapper e) {
    if (e.getSourceVertex().is_infinite() && e.getNextEdge().getSourceVertex().is_infinite()) return;
    if (isEdgeInvalid(e)){
        size_t node0_index = e.getFace().getDelaunayNodeIndex();
        size_t node1_index = e.getTwinEdge().getFace().getDelaunayNodeIndex();
        DCEL::EdgeWrapper newE = dcel.flip_edge(e);

        std::vector<size_t> old_leafs_indexes = {node0_index, node1_index};
        std::vector<DCEL::FaceWrapper> new_faces = {newE.getFace(), newE.getTwinEdge().getFace()};
        std::vector<size_t> new_leafs_indexes = tree.insert(new_faces, old_leafs_indexes); //у меня реализовано что сразу для двух делается
        //см. комменатрии под tree.insert чтобы понять что тут происходит и почему именно так((((
        for (int i = 0; i < new_leafs_indexes.size(); ++i) {
            dcel.update_face_node_index(new_faces[i], new_leafs_indexes[i]);
        }

        validateEdge(newE.getNextEdge());
        validateEdge(newE.getPrevEdge());
        validateEdge(newE.getTwinEdge().getNextEdge());
        validateEdge(newE.getTwinEdge().getPrevEdge());
    }
}


bool Triangulation::DelaunayTriangulation::isEdgeInvalid(DCEL::EdgeWrapper e) {
    if (e.getSourceVertex().is_infinite() || e.getNextEdge().getSourceVertex().is_infinite()) {
        //наше ребро инцидентно одной бесокнечной вершине, двум уже не может т.к. отсеили до вызова этого метода

        DCEL::EdgeWrapper e_twin = e.getTwinEdge();

        // ищем конечное ребро в twin-грани
        DCEL::EdgeWrapper finite_edge = e_twin; //перезапишется
        {
            DCEL::EdgeWrapper iter = e_twin;
            for (int attempt = 0; attempt < 3; ++attempt) {
                if (!iter.getSourceVertex().is_infinite() && !iter.getNextEdge().getSourceVertex().is_infinite()) {
                    finite_edge = iter;
                    break;
                }
                iter = iter.getNextEdge();
            }
        }
        DCEL::VertexWrapper D = e.getPrevEdge().getSourceVertex();
        if (D.is_infinite()) return false; // если D бесконечна — флип невозможен и не нужен, вроде как не очень возможна такая ситуация, но лишнем не будет

        Point_2 finiteSource  = finite_edge.getSourceVertex().getGeometry();
        Point_2 finiteTarget  = finite_edge.getNextEdge().getSourceVertex().getGeometry();
        Point_2 pd  = D.getGeometry(); //противолежащая

        return vector_s::vector_orientation(finiteSource, finiteTarget, pd) == vector_s::orientation::left;
    }

    // Оба конца конечны — стандартный in_circle
    return vector_s::in_circle(
            e.getTwinEdge().getSourceVertex().getGeometry(),
            e.getTwinEdge().getNextEdge().getSourceVertex().getGeometry(),
            e.getTwinEdge().getPrevEdge().getSourceVertex().getGeometry(),
            e.getPrevEdge().getSourceVertex().getGeometry());
}


void Triangulation::DelaunayTriangulation::triangulate() {
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

Triangulation::DelaunayTriangulation::DelaunayTriangulation(std::vector<Point_2> &pts) {
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

