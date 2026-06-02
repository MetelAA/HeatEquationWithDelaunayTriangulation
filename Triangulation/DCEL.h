#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H

#include <vector>
#include <cstdio>
#include "Point_2.h"
#include <optional>
#include "../magic_constants.h"
#include "../DTO.h"


class DCEL {
private:
    class vertex {
    public:
        vertex() {}

        vertex(size_t pointCoordsIndex) {}

        vertex(size_t pointCoordsIndex, size_t randomEdgeIndex) : point_coords_index(pointCoordsIndex),
                                                                  random_edge_index(randomEdgeIndex)
        {}

        size_t point_coords_index; // индекс координат точки(Point_2) в vector coords, для p_inf тут будет не валидное число из магических констант!
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

        face(size_t edgeIndex, size_t nodeIndex) : edge_index(edgeIndex), node_index(nodeIndex) {}

        size_t edge_index; //индекс случайного (условно) ребра входящего в эту грань, через него можно найти все остальные
        size_t node_index; //индекс ноды из delaunay_tree в которую входит эта грань, даже фактически не ноды а листа
    };

    std::vector<vertex> vertexes;
    std::vector<Point_2> coords;
    std::vector<edge> edges;
    std::vector<face> faces;

public:
    class FaceWrapper;

    class VertexWrapper;

    class EdgeWrapper;

    class VertexWrapper {
        friend DCEL;
    public:
        VertexWrapper(size_t vertexIndex, DCEL *dcel) : vertex_index(vertexIndex), dcel(dcel) {}

        bool operator==(const VertexWrapper &other) const {
            return vertex_index == other.vertex_index;
        }

        bool operator!=(const VertexWrapper &other) const {
            return !(*this == other);
        }

        bool is_infinite() const {
            return dcel->vertexes[vertex_index].point_coords_index == Constants::p_inf;
        }

        EdgeWrapper getEdge() const {
            vertex curVertex = dcel->vertexes[vertex_index];
            return EdgeWrapper(curVertex.random_edge_index, dcel);
        }

        Point_2 getGeometry() const {
            vertex curVertex = dcel->vertexes[vertex_index];
            return dcel->coords[curVertex.point_coords_index];
        }



    private:

        size_t vertex_index; // индекс вершины в vector vertexes которую оборачивает в этот wrapper
        DCEL *dcel;
    };

    class EdgeWrapper {
        friend DCEL;
    public:
        EdgeWrapper(size_t edgeIndex, DCEL *dcel) : edge_index(edgeIndex), dcel(dcel) {}

        bool operator==(const EdgeWrapper &other) const {
            return edge_index == other.edge_index;
        }

        bool operator!=(const EdgeWrapper &other) const {
            return !(*this == other);
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

        size_t edge_index; // индекс ребра в vector vertexes которую оборачивает в этот wrapper
        DCEL *dcel;
    };

    class FaceWrapper {
        friend DCEL;
    public:
        FaceWrapper(size_t faceIndex, DCEL *dcel) : face_index(faceIndex), dcel(dcel) {}

        EdgeWrapper getEdge() const {
            face curFace = dcel->faces[face_index];
            return EdgeWrapper(curFace.edge_index, dcel);
        }

        size_t getDelaunayNodeIndex() const{
            return dcel->faces[face_index].node_index;
        }

    private:
        size_t face_index; // индекс грани в vector vertexes которую оборачивает в этот wrapper
        DCEL *dcel;
    };

    DCEL::EdgeWrapper flip_edge(EdgeWrapper p_i_p_j_edge); //достанем из враппера индекс сами, в ручную

    void update_face_node_index(FaceWrapper face, size_t node_index);
    //т.к. везде работаем с копиями или ссылками непойми откуда, то сюда нужно будет передать wrapper, тут внутри мы можем посмтреть на какую face из
    //вектора faces он ссылается т.к. face_index - private и установить соответсвующей face такой nodeIndex

    VertexWrapper split_face(FaceWrapper faceWithPointOn, std::optional<EdgeWrapper> edgeWithPointOn,
                             Point_2 point); //возвращаем обёртку на вершину которой сплитанули, то ребро которые optional используется только тогда, когда мы попали на ребро!

    std::vector<FaceWrapper> get_incident_faces(
            VertexWrapper v) const; //выдаёт все инцидентные грани для вершины после split_face нужно запускать для той же вершины для которой запускали split_face, т.е.
    // враппер которой получили как результата выполнения метода split_face

    std::vector<EdgeWrapper> get_outgoing_edges(VertexWrapper v) const;

    std::vector<DCEL::FaceWrapper> init_dcel_with_big_inf_triangle(size_t coordsSize);

    std::vector<DCEL::VertexWrapper> getVertexesInWrappers();
    DTO::TriangulationResult getTriangulationWithCorrectBoundary();
    //содержит мапу, где храняться все ребра образующие границу, т.к. ребра у нас направленные,
    //то содержит обход границы против часовой стрелки, ключом мапы выступает исток ребра, значением - сток
};




#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H