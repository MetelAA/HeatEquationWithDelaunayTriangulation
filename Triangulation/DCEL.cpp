#include <stdexcept>
#include <queue>
#include "DCEL.h"

DCEL::VertexWrapper
DCEL::split_face(DCEL::FaceWrapper faceWithPointOn, std::optional<EdgeWrapper> edgeWithPointOn, Point_2 point) {
    if (edgeWithPointOn.has_value()) {
        //случай с попаданием на ребро
        edge e3 = this->edges[edgeWithPointOn.value().getCurrentEdgeIndex()]; //e3, e4 - ребра на котороые попала точка, ребра по которым разделяются face0, face1
        edge e4 = this->edges[e3.twin_edge_index];
        size_t e3_index = edgeWithPointOn.value().getCurrentEdgeIndex(), e4_index = e3.twin_edge_index;

        //соответсвенно к e3 относиться к face0, а e4 относиться к face1
        face face0 = this->faces[e3.face_index];
        face face1 = this->faces[e4.face_index];
        size_t face0_index = e3.face_index, face1_index = e4.face_index;

        //найдем все edge с face0
        edge e1 = this->edges[e3.next_edge_index];
        edge e2 = this->edges[e3.prev_edge_index];

        //найдём все edge с face1
        edge e5 = this->edges[e4.next_edge_index];
        edge e6 = this->edges[e4.prev_edge_index];

        size_t e1_index = e3.next_edge_index, e2_index = e3.prev_edge_index, e5_index = e4.next_edge_index, e6_index = e4.prev_edge_index;

        //подтащим текущие точки
        vertex p_i = this->vertexes[e1.source_vertex_index];
        vertex p_j = this->vertexes[e2.source_vertex_index];
        vertex p_k = this->vertexes[e5.source_vertex_index];
        vertex p_l = this->vertexes[e6.source_vertex_index];
        size_t p_i_index = e1.source_vertex_index, p_j_index = e2.source_vertex_index, p_k_index = e5.source_vertex_index, p_l_index = e6.source_vertex_index;

        //создадим новую точку p_r
        vertex p_r;
        p_r.point_coords_index = this->coords.size();
        this->coords.push_back(point);
        size_t p_r_index = this->vertexes.size(); //не забыть в конце добавить!!!

        //создаём все новые ребра, чёт много конечно((((

        edge e7, e8, e9, e10, e11, e12, e13, e14;
        size_t edge_number = this->edges.size() - 1;
        size_t e7_index = e3_index, e8_index = e4_index; //переиспользуем старые индексы в векторе ребер от e3 и e4
        size_t e9_index = edge_number + 1, e10_index = edge_number + 2, e11_index = edge_number + 3,
                e12_index = edge_number + 4, e13_index = edge_number + 5, e14_index = edge_number + 6;

        { // заполняем актуальные данные для e7
            e7.next_edge_index = e8_index;
            e7.prev_edge_index = e1_index;
            e7.twin_edge_index = e10_index;
            e7.source_vertex_index = p_j_index;
            //face (грани) тоже пока не заполянем
            //дату не заполняем (data)
        }

        { // заполняем актуальные данные для e8
            e8.next_edge_index = e1_index;
            e8.prev_edge_index = e7_index;
            e8.twin_edge_index = e14_index;
            e8.source_vertex_index = p_r_index;
        }

        { // заполняем актуальные данные для e9
            e9.next_edge_index = e10_index;
            e9.prev_edge_index = e2_index;
            e9.twin_edge_index = e12_index;
            e9.source_vertex_index = p_k_index;
        }

        { // заполняем актуальные данные для e10
            e10.next_edge_index = e2_index;
            e10.prev_edge_index = e9_index;
            e10.twin_edge_index = e7_index;
            e10.source_vertex_index = p_r_index;
        }

        { // заполняем актуальные данные для e11
            e11.next_edge_index = e12_index;
            e11.prev_edge_index = e5_index;
            e11.twin_edge_index = e13_index;
            e11.source_vertex_index = p_l_index;
        }

        { // заполняем актуальные данные для e12
            e12.next_edge_index = e5_index;
            e12.prev_edge_index = e11_index;
            e12.twin_edge_index = e9_index;
            e12.source_vertex_index = p_r_index;
        }

        { // заполняем актуальные данные для e13
            e13.next_edge_index = e6_index;
            e13.prev_edge_index = e14_index;
            e13.twin_edge_index = e11_index;
            e13.source_vertex_index = p_r_index;
        }

        { // заполняем актуальные данные для e14
            e14.next_edge_index = e13_index;
            e14.prev_edge_index = e6_index;
            e14.twin_edge_index = e8_index;
            e14.source_vertex_index = p_i_index;
        }

        {//изменим prev/next на акутальные для новых граней у старых ребер, изменим у e1
            e1.next_edge_index = e7_index;
            e1.prev_edge_index = e8_index;
        }

        {//изменим prev/next на акутальные для новых граней у старых ребер, изменим у e2
            e2.next_edge_index = e9_index;
            e2.prev_edge_index = e10_index;
        }

        {//изменим prev/next на акутальные для новых граней у старых ребер, изменим у e5
            e5.next_edge_index = e11_index;
            e5.prev_edge_index = e12_index;
        }

        {//изменим prev/next на акутальные для новых граней у старых ребер, изменим у e6
            e6.next_edge_index = e14_index;
            e6.prev_edge_index = e13_index;
        }
        //e3, e4 будут удалены так что у них ничего менять не надо

        //добавим новые грани, face2, face3, face4, face5 которые образуют новые ребра, т.к. face0, face1 удаляем, то их индексы переназначим face2 и face3 соответсвенно
        face face2, face3, face4, face5;
        size_t face_number = this->faces.size() - 1;
        size_t face2_index = face0_index, face3_index = face1_index, face4_index = face_number + 1, face5_index =
                face_number + 2;

        //заполняем данными, на дату (data) такде плевать
        face2.edge_index = e1_index;
        face3.edge_index = e2_index;
        face4.edge_index = e5_index;
        face5.edge_index = e6_index;

        //заполняем индексы граней на ребрах (всех), ну кроме удалённых e3, e4
        {
            e1.face_index = face2_index;
            e2.face_index = face3_index;
            e5.face_index = face4_index;
            e6.face_index = face5_index;
            e7.face_index = face2_index;
            e8.face_index = face2_index;
            e9.face_index = face3_index;
            e10.face_index = face3_index;
            e11.face_index = face4_index;
            e12.face_index = face4_index;
            e13.face_index = face5_index;
            e14.face_index = face5_index;
        }

        p_r.random_edge_index = e8_index; //закидываем случайный номер исходяшего ребра в новую вершину

        //обновим в старых вершинах исходящие ребра во избежание указания на удалённое ребро (его могло заменить ребро которое не исходит из этой вершины)
        //на границе раздела face0 и face1 лежат две точки p_i, p_k
        this->vertexes[p_i_index].random_edge_index = e14_index;
        this->vertexes[p_k_index].random_edge_index = e9_index;

        this->edges[e3_index] = e7;
        this->edges[e4_index] = e8; //закинули по теми же индексами
        this->edges.push_back(e9);
        this->edges.push_back(e10);
        this->edges.push_back(e11);
        this->edges.push_back(e12);
        this->edges.push_back(e13);
        this->edges.push_back(e14);

        this->vertexes.push_back(p_r);

        this->faces[face0_index] = face2;
        this->faces[face1_index] = face3;
        this->faces.push_back(face4);
        this->faces.push_back(face5);


        return VertexWrapper(p_r_index, this);
    } else {
        //случай с попаданием внутрь грани
        face face0 = this->faces[faceWithPointOn.getCurrentFaceIndex()];
        size_t face0_index = faceWithPointOn.getCurrentFaceIndex();
        edge e0 = this->edges[face0.edge_index];
        edge e1 = this->edges[e0.next_edge_index];
        edge e2 = this->edges[e0.prev_edge_index];
        size_t e0_index = face0.edge_index, e1_index = e0.next_edge_index, e2_index = e0.prev_edge_index;
        vertex p_i = this->vertexes[e0.source_vertex_index];
        vertex p_j = this->vertexes[e1.source_vertex_index];
        vertex p_k = this->vertexes[e2.source_vertex_index];
        size_t p_i_index = e0.source_vertex_index, p_j_index = e1.source_vertex_index, p_k_index = e2.source_vertex_index;

        //добавляем новую веришну p_r
        vertex p_r;
        p_r.point_coords_index = this->coords.size();
        this->coords.push_back(point);
        size_t p_r_index = this->vertexes.size(); //не забыть в конце закинуть туда p_r

        //создаём всю гамму новых ребер
        size_t edge_number = this->edges.size() - 1;
        size_t e3_index = edge_number + 1, e4_index = edge_number + 2, e5_index = edge_number + 3, e6_index =
                edge_number + 4, e7_index = edge_number + 5, e8_index = edge_number + 6;
        edge e3, e4, e5, e6, e7, e8;

        { // заполняем актуальные данные для e3
            e3.next_edge_index = e4_index;
            e3.prev_edge_index = e0_index;
            e3.twin_edge_index = e8_index;
            e3.source_vertex_index = p_j_index;
            //face (грани) тоже пока не заполянем
            //дату не заполняем (data)
        }

        { // заполняем актуальные данные для e4
            e4.next_edge_index = e0_index;
            e4.prev_edge_index = e3_index;
            e4.twin_edge_index = e5_index;
            e4.source_vertex_index = p_r_index;
        }

        {// заполняем актуальные данные для e5
            e5.next_edge_index = e6_index;
            e5.prev_edge_index = e2_index;
            e5.twin_edge_index = e4_index;
            e5.source_vertex_index = p_i_index;
        }

        {// заполняем актуальные данные для e6
            e6.next_edge_index = e2_index;
            e6.prev_edge_index = e5_index;
            e6.twin_edge_index = e7_index;
            e6.source_vertex_index = p_r_index;
        }

        {// заполняем актуальные данные для e7
            e7.next_edge_index = e8_index;
            e7.prev_edge_index = e1_index;
            e7.twin_edge_index = e6_index;
            e7.source_vertex_index = p_k_index;
        }

        {// заполняем актуальные данные для e8
            e8.next_edge_index = e1_index;
            e8.prev_edge_index = e7_index;
            e8.twin_edge_index = e3_index;
            e8.source_vertex_index = p_r_index;
        }

        { //изменяем next/prev у e0
            e0.next_edge_index = e3_index;
            e0.prev_edge_index = e4_index;
        }

        {//изменяем next/prev у e1
            e1.next_edge_index = e7_index;
            e1.prev_edge_index = e8_index;
        }

        {//изменяем next/prev у e2
            e2.next_edge_index = e5_index;
            e2.prev_edge_index = e6_index;
        }

        face face1, face2, face3;
        size_t face_number = this->faces.size() - 1;
        size_t face1_index = face0_index, face2_index = face_number + 1, face3_index = face_number +
                                                                                       2; //т.к. face0 удаляем, то на её место запихнём face1 (не забыть в конце впихнуть), остальные две - новые
        face1.edge_index = e0_index;
        //дату также не заполянем, в прицнипе не нужна она, как оказывается)))
        face2.edge_index = e2_index;
        face3.edge_index = e1_index;


        { //заполняем индексы граней на ребрах
            e0.face_index = face1_index;
            e1.face_index = face3_index;
            e2.face_index = face2_index;
            e3.face_index = face1_index;
            e4.face_index = face1_index;
            e5.face_index = face2_index;
            e6.face_index = face2_index;
            e7.face_index = face3_index;
            e8.face_index = face3_index;
        }

        p_r.random_edge_index = e4_index; //закидываем случайный номер исходяшего ребра в новую вершину

        this->edges.push_back(e3);
        this->edges.push_back(e4);
        this->edges.push_back(e5);
        this->edges.push_back(e6);
        this->edges.push_back(e7);
        this->edges.push_back(e8);

        this->vertexes.push_back(p_r);

        this->faces[face0_index] = face1;
        this->faces.push_back(face2);
        this->faces.push_back(face3);

        return VertexWrapper(p_r_index, this);
    }


}

std::vector<DCEL::FaceWrapper> DCEL::get_incident_faces(DCEL::VertexWrapper v) const {
    std::vector<FaceWrapper> incident_faces;
    EdgeWrapper start = v.getEdge();
    EdgeWrapper iter = start;
    do {
        incident_faces.push_back(iter.getFace());
        iter = iter.getTwinEdge().getNextEdge();
    } while (iter != start);
    return incident_faces;
}


std::vector<DCEL::EdgeWrapper> DCEL::get_outgoing_edges(DCEL::VertexWrapper v) const {
    std::vector<EdgeWrapper> outgoing_edges;
    EdgeWrapper start = v.getEdge();
    EdgeWrapper iter = start;

    do {
        outgoing_edges.push_back(iter);
        iter = iter.getTwinEdge().getNextEdge();
    } while (iter != start);
    return outgoing_edges;
}

void DCEL::update_face_node_index(DCEL::FaceWrapper face, size_t node_index) {
    this->faces[face.face_index].node_index = node_index;
}


DCEL::FaceWrapper DCEL::init_dcel_with_big_inf_triangle(const Point_2 &p0) {
    coords.push_back(p0);

    const size_t inf_left_top_index = 0;
    const size_t inf_right_bottom_index = 1;
    const size_t p0_index = 2;

    vertexes.emplace_back(Constants::p_inf_left_top_index, 0); // 0 индекс исходящего ребра из верхней левой веришны
    vertexes.emplace_back(Constants::p_inf_right_bottom_index, 1); // 1 индекс исходящего ребра из правой нижней веришны
    vertexes.emplace_back(0, 2); // 2 индекс исходящего ребра из первой реально вершины (точки)

    // создаём рёбра (обход: inf_left_top -> inf_right_bottom -> p0 -> inf_left_top)
    const size_t e0_index = 0, e1_index = 1, e2_index = 2;

    edges.emplace_back(e1_index, e2_index, Constants::invalid_twin_edge, inf_left_top_index, 0);
    edges.emplace_back(e2_index, e0_index, Constants::invalid_twin_edge, inf_right_bottom_index, 0);
    edges.emplace_back(e0_index, e1_index, Constants::invalid_twin_edge, p0_index, 0);

    faces.emplace_back(e0_index);
    faces[0].node_index = 0;   // связь с корневым листом дерева (заранее чтобы потом не пробрасывать)

    return FaceWrapper(0, this);
}


DCEL::EdgeWrapper DCEL::flip_edge(EdgeWrapper p_i_p_j_edge) {
    //короче, изнчально этот метод был legalize_edge но я потом понял что он не тут долежн быть, увы, так что тут теперь flip_edge и я надеюсь что он правильные, проверить!!!!!!!!


    //проинициализируем всё ребра входящие в ОБЕ!!! грани (face0 face1)
    edge e0 = this->edges[p_i_p_j_edge.edge_index];
    edge e3 = this->edges[e0.twin_edge_index]; //twin ребра есть у всех, кроме трёх ребер входящих в изначальный треугольник, где реальные координаты только у точки p_0
    // и бесконечные у p_-1, p_-2, это нужно написать отдельно! (до этого иначе грохнется) хз можем ли мы попасть в flip_edge с ребром которое соединяет или две бесконечные или одну бесоконечную и конечную вершины
    size_t e0_index = p_i_p_j_edge.edge_index, e3_index = e0.twin_edge_index;

    // найдём 3 ребра образующих face0 (p_i, p_j, p_r)
    //координаты вроде реальные у всех, все вершины с несуществующими координатами обрабатываем до этого в самом начале
    vertex p_i = this->vertexes[e0.source_vertex_index];
    vertex p_j = this->vertexes[this->edges[e0.next_edge_index].source_vertex_index]; // через следующее ребро
    vertex p_r = this->vertexes[this->edges[e0.prev_edge_index].source_vertex_index]; // через предыдущее ребро
    vertex p_k = this->vertexes[this->edges[e3.prev_edge_index].source_vertex_index]; //в e3 входит ребро p_k p_j, у него source как раз и есть наша p_r
    size_t p_i_index = e0.source_vertex_index, p_j_index = this->edges[e0.next_edge_index].source_vertex_index,
            p_r_index = this->edges[e0.prev_edge_index].source_vertex_index, p_k_index = this->edges[e3.prev_edge_index].source_vertex_index;

    //доинициализируем все старые ребра
    edge e1 = this->edges[e0.next_edge_index];
    edge e2 = this->edges[e0.prev_edge_index];
    edge e4 = this->edges[e3.next_edge_index];
    edge e5 = this->edges[e3.prev_edge_index];
    size_t e1_index = e0.next_edge_index, e2_index = e0.prev_edge_index, e4_index = e3.next_edge_index, e5_index = e3.prev_edge_index;

    //создаём два новых ребра
    edge e6, e7;
    size_t e6_index = e0_index, e7_index = e3_index; //индексы двух новосозданных соответствуют индкас двух удалённых, у e6 индекс e0, у e7 индекс e3 (и никак иначе)

    {//забиваем правильными параметрами e6
        e6.next_edge_index = e5_index;
        e6.prev_edge_index = e1_index;
        e6.twin_edge_index = e7_index;
        e6.source_vertex_index = p_r_index;
        //дату как обычно не трогаем
    }

    {//забиваем правильными параметрами e6
        e7.next_edge_index = e2_index;
        e7.prev_edge_index = e4_index;
        e7.twin_edge_index = e6_index;
        e7.source_vertex_index = p_k_index;
        //дату как обычно не трогаем
    }

    //теперь обновим инфу у старых
    {
        e1.next_edge_index = e6_index;
        e1.prev_edge_index = e5_index;
    }

    {
        e2.next_edge_index = e4_index;
        e2.prev_edge_index = e7_index;
    }

    {
        e4.next_edge_index = e7_index;
        e4.prev_edge_index = e2_index;
    }

    {
        e5.next_edge_index = e1_index;
        e5.prev_edge_index = e6_index;
    }

    //теперь создаим новые плоскости face2, face3 на замен старых face0 face1, при этом индекс face2 = индексу face0, индекс face3 = индексу face1 (тут не принципиально, но будет так)

    face face2, face3;
    face2.edge_index = e1_index;
    face3.edge_index = e2_index;
    size_t face2_index = e0.face_index, face3_index = e3.face_index;


    //заполни индексы граней ребрам
    {
        e1.face_index = face2_index;
        e2.face_index = face3_index;
        e4.face_index = face3_index;
        e5.face_index = face2_index;
        e6.face_index = face2_index;
        e7.face_index = face3_index;
    }

    //теперь обнвоим всё в списках
    this->edges[e0_index] = e6;
    this->edges[e1_index] = e1;
    this->edges[e2_index] = e2;
    this->edges[e3_index] = e7;
    this->edges[e4_index] = e4;
    this->edges[e5_index] = e5;

    this->faces[face2_index] = face2;
    this->faces[face3_index] = face3;

    this->vertexes[p_i_index].random_edge_index = e4_index;
    this->vertexes[p_j_index].random_edge_index = e1_index;

    return EdgeWrapper(p_i_p_j_edge.edge_index,
                       this); //он типо даже так то не поменялся, но для корректности лучше вернуть новый
}


std::vector<DCEL::VertexWrapper> DCEL::getVertexesInWrappers() {
    std::vector<DCEL::VertexWrapper> wrappers;
    for (int i = 0; i < this->vertexes.size(); ++i) {
        wrappers.emplace_back(i, this);
    }
    return wrappers;
}


void DCEL::getTriangulationWithCorrectBoundary(const std::unordered_map<Point_2, Point_2> &boundary_vertexes_map) {

    //сделаем обход граней, нужно найти все внешние грани
    std::queue<DCEL::FaceWrapper> externalFaces;
    std::vector<bool> visitedFaces(this->faces.size(), false);

    //две вершины с отрицательными индексами коориднат (бесконечные) всегда лежат на индексах 1 и 0 в векторе вершин.
    //индекс = 0
    for (DCEL::EdgeWrapper edge : this->get_outgoing_edges(this->getVertexesInWrappers()[0])) {
        if (!visitedFaces[edge.getFace().getCurrentFaceIndex()]) {
            visitedFaces[edge.getFace().getCurrentFaceIndex()] = true;
            externalFaces.push(edge.getFace());
        }
        if (edge.hasValidTwin()) {
            if (!visitedFaces[edge.getTwinEdge().getFace().getCurrentFaceIndex()]) {
                visitedFaces[edge.getTwinEdge().getFace().getCurrentFaceIndex()] = true;
                externalFaces.push(edge.getTwinEdge().getFace());
            }
        }
    }

    //индекс = 1
    for (DCEL::EdgeWrapper edge : this->get_outgoing_edges(this->getVertexesInWrappers()[1])) {
        if (!visitedFaces[edge.getFace().getCurrentFaceIndex()]) {
            visitedFaces[edge.getFace().getCurrentFaceIndex()] = true;
            externalFaces.push(edge.getFace());
        }
        if (edge.hasValidTwin()) {
            if (!visitedFaces[edge.getTwinEdge().getFace().getCurrentFaceIndex()]) {
                visitedFaces[edge.getTwinEdge().getFace().getCurrentFaceIndex()] = true;
                externalFaces.push(edge.getTwinEdge().getFace());
            }
        }
    }

    //запистим обход
    while (!externalFaces.empty()) {
        DCEL::FaceWrapper face = externalFaces.front();
        externalFaces.pop();
        DCEL::EdgeWrapper start_edge = face.getEdge();
        DCEL::EdgeWrapper iter_edge = start_edge;

        do {
            if (!iter_edge.getSourceVertex().is_infinite()) {
                Point_2 sourceVertexCoords = iter_edge.getSourceVertex().getGeometry();
                Point_2 targetVertexCoords = iter_edge.getNextEdge().getSourceVertex().getGeometry();

                bool isBoundary = false;

                if (boundary_vertexes_map.count(sourceVertexCoords) != 0  // прямое направление
                    && boundary_vertexes_map.at(sourceVertexCoords) == targetVertexCoords) {
                    isBoundary = true;
                } else if (boundary_vertexes_map.count(targetVertexCoords) != 0 // обратное направление
                           && boundary_vertexes_map.at(targetVertexCoords) == sourceVertexCoords) {
                    isBoundary = true;
                }

                if (!isBoundary) {
                    DCEL::EdgeWrapper iter_edge_twin = iter_edge.getTwinEdge();
                    if (!visitedFaces[iter_edge_twin.getFace().getCurrentFaceIndex()]) {
                        visitedFaces[iter_edge_twin.getFace().getCurrentFaceIndex()] = true;
                        externalFaces.push(iter_edge_twin.getFace());
                    }
                }
            }
            iter_edge = iter_edge.getNextEdge();
        } while (start_edge != iter_edge);
    }

    std::vector<DCEL::FaceWrapper> notVisitedFaces;

    for (size_t i = 0; i < this->faces.size(); ++i) {
        if (!visitedFaces[i])
            notVisitedFaces.emplace_back(i, this);
    }


}