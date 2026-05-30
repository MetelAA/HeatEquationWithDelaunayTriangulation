#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H

#include <vector>
#include <cstdio>
#include "point_2.h"
#include <optional>
#include <unordered_map>
#include "magic_constants.h"
#include "dto.h"


class dcel {
private:
    class vertex {
    public:
        vertex() {}

        vertex(size_t pointCoordsIndex, size_t randomEdgeIndex) : point_coords_index(pointCoordsIndex),
                                                                  random_edge_index(randomEdgeIndex)
        {}

        size_t point_coords_index; // индекс координат точки(Pint_2) в vector coords, для p-1, p-2 тут будут два не валидных магических числа из констант!
        //Почему не использовал optional тут, ну типо доп нагрузка в дата классе, где гораздо! чаще вершины с корректным индексом, плюс тут два магических значения разного значения, мы по разному обрабатываем
        //верхнее и левое на бесконечностях, так что да, я бы вообще тут чисто всё enum-чиками орагнизовал, но увы(((
        size_t random_edge_index; //индекс случайного (условно) ребра в vector edges которое исходит из этой вершины, через него можно найти все остальные
    };

    class edge {
    public:
        edge() {}

        edge(size_t nextEdgeIndex, size_t prevEdgeIndex, size_t twinEdgeIndex, size_t sourceVertexIndex,
             size_t faceIndex) : next_edge_index(nextEdgeIndex), prev_edge_index(prevEdgeIndex),
                                 twin_edge_index(twinEdgeIndex), source_vertex_index(sourceVertexIndex),
                                 face_index(faceIndex){}

        size_t next_edge_index; //next_edge_in_current_face_index индекс следующего ребра в той грани в которую входит это ребро, всегда если триангуляции в моменте корректна
        size_t prev_edge_index; //prev_edge_in_current_face_index тож самое что сверху ток предыдыущее
        size_t twin_edge_index; //индекс братана с обратным направлением (у которого source = вершине в которое мы входим), есть не всегда! Придумаю какие-нибудь магические чилса для обозначения
        size_t source_vertex_index; //индекс вершины из которой исходит ребро
        size_t face_index; //инедкс грани к которой принадлежит ребро
    };

    class face {
    public:
        face() {}

        face(size_t edgeIndex) : edge_index(edgeIndex) {}

        size_t edge_index; //индекс случайного (условно) ребра входящего в эту грань, через него можно найти все остальные
        size_t node_index; //индекс ноды из delaunay_tree в которую входит эта грань, даже фактически не ноды а листа
    };

    std::vector<vertex> vertexes;
    std::vector<point_2> coords;
    std::vector<edge> edges;
    std::vector<face> faces;

public:
    class FaceWrapper;

    class VertexWrapper;

    class EdgeWrapper;

    class VertexWrapper {
        friend dcel;
    public:
        VertexWrapper(size_t vertexIndex, dcel *dcel) : vertex_index(vertexIndex), dcel(dcel) {}

        bool operator==(const VertexWrapper &other) const {
            return vertex_index == other.vertex_index;
        }

        bool operator!=(const VertexWrapper &other) const {
            return !(*this == other);
        }

        bool is_infinite() const {
            return dcel->vertexes[vertex_index].point_coords_index == Constants::p_inf_left_top_index ||
                   dcel->vertexes[vertex_index].point_coords_index == Constants::p_inf_right_bottom_index;
        }

        EdgeWrapper getEdge() const {
            vertex curVertex = dcel->vertexes[vertex_index];
            return EdgeWrapper(curVertex.random_edge_index, dcel);
        }

        //мб добавить возврат типа ребра, типо нормал, или те которые на бесконечности (по отдельному типу для каждого!)
        point_2 getGeometry() const {
            vertex curVertex = dcel->vertexes[vertex_index];
            return dcel->coords[curVertex.point_coords_index];
        }



    private:
        size_t getCurrentVertexIndex() const {
            return vertex_index;
        }

        size_t vertex_index; // индекс вершины в vector vertexes которую оборачивает в этот wrapper
        dcel *dcel;
    };

    class EdgeWrapper {
        friend dcel;
    public:
        EdgeWrapper(size_t edgeIndex, dcel *dcel) : edge_index(edgeIndex), dcel(dcel) {}

        bool operator==(const EdgeWrapper &other) const {
            return edge_index == other.edge_index;
        }

        bool operator!=(const EdgeWrapper &other) const {
            return !(*this == other);
        }

        bool hasValidTwin() const {
            return dcel->edges[edge_index].twin_edge_index != Constants::invalid_twin_edge;
        }

        EdgeWrapper getNextEdge() const {
            edge &curEdge = dcel->edges[edge_index];
            return EdgeWrapper(curEdge.next_edge_index, dcel);
        }

        EdgeWrapper getPrevEdge() const {
            edge &curEdge = dcel->edges[edge_index];
            return EdgeWrapper(curEdge.prev_edge_index, dcel);
        }

        EdgeWrapper getTwinEdge() const {
            edge &curEdge = dcel->edges[edge_index];
            return EdgeWrapper(curEdge.twin_edge_index, dcel);
        }

        VertexWrapper getSourceVertex() const {
            edge &curEdge = dcel->edges[edge_index];
            return VertexWrapper(curEdge.source_vertex_index, dcel);
        }

        FaceWrapper getFace() const {
            edge &curEdge = dcel->edges[edge_index];
            return FaceWrapper(curEdge.face_index, dcel);
        }

    private:

        size_t getCurrentEdgeIndex() const{
            return edge_index;
        }

        size_t edge_index; // индекс ребра в vector vertexes которую оборачивает в этот wrapper
        dcel *dcel;
    };

    class FaceWrapper {
        friend dcel;
    public:
        FaceWrapper(size_t faceIndex, dcel *dcel) : face_index(faceIndex), dcel(dcel) {}

        EdgeWrapper getEdge() const {
            face curFace = dcel->faces[face_index];
            return EdgeWrapper(curFace.edge_index, dcel);
        }

        size_t getDelaunayNodeIndex() const{
            return dcel->faces[face_index].node_index;
        }

    private:
        size_t getCurrentFaceIndex() const {
            return face_index;
        }

        size_t face_index; // индекс грани в vector vertexes которую оборачивает в этот wrapper
        dcel *dcel;
    };

    dcel::EdgeWrapper flip_edge(EdgeWrapper p_i_p_j_edge); //достанем из враппера индекс сами, в ручную

    void update_face_node_index(FaceWrapper face, size_t node_index); //т.к. везде работаем с копиями или ссылками непойми откуда, то сюда нужно будет передать wrapper, тут внутри мы можем посмтреть на какую face из
    //вектора faces он ссылается т.к. face_index - private и установить соответсвующей face такой nodeIndex

    VertexWrapper split_face(FaceWrapper faceWithPointOn, std::optional<EdgeWrapper> edgeWithPointOn,
                             point_2 point); //возвращаем обёртку на вершину которой сплитанули, то ребро которые optional используется только тогда, когда мы попали на ребро!

    std::vector<FaceWrapper> get_incident_faces(
            VertexWrapper v) const; //выдаёт все инцидентные грани для вершины после split_face нужно запускать для той же вершины для которой запускали split_face, т.е.
    // враппер которой получили как результата выполнения метода split_face

    std::vector<EdgeWrapper> get_outgoing_edges(VertexWrapper v) const;

    dcel::FaceWrapper init_dcel_with_big_inf_triangle(const point_2& p0); //начальная точка, которая самая верхняя и самая правая из самых верхних, возращаем обёртку на первую вершину чтобы потом закинуть её корреткный node_index
    //возвращаем корневую грань (первую), которая на двух бесконечных и одной конечной

    std::vector<dcel::VertexWrapper> getVertexesInWrappers();
    dto::TriangulationResult getTriangulationWithCorrectBoundary(const std::unordered_map<point_2, point_2>& boundary_vertexes_map);
    //содержит мапу, где храняться все ребра образующие границу, т.к. ребра у нас направленные,
    //то содержит обход границы против часовой стрелки, ключом мапы выступает исток ребра, значением - сток
};




#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H