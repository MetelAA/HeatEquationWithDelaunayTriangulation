#ifndef HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H
#define HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H


#include <vector>
#include <cstdio>
#include "Point_2.h"


template<class VertexData = size_t, class EdgeData = size_t, class FaceData = size_t>
class DCEL { //DCEL = doubly connected edge list, оболочка для хранения всех актуальных (ребер, граней и вершин) триангуляции


public:

    class VertexWrapper{ //враперы пока оставим тут пока не понятно какой сюда функицонал добавлять(
    public:
        
        const VertexData& getData(){
            return dcel->vertexes[vertex_index].data;
        }

        const Point_2& getGeometry() const {
            return dcel->vertexes[vertex_index].point;
        }

    private:
        size_t vertex_index; // current_vertex_index
        DCEL* dcel;
    };

    class EdgeWrapper{
    public:
        EdgeWrapper(size_t edgeIndex, DCEL *dcel) : edge_index(edgeIndex), dcel(dcel) {}

        const EdgeData& getData(){
            return dcel->edges[edge_index].data;
        }

    private:
        size_t edge_index; // current_edge_index
        DCEL* dcel;
    };

    class FaceWrapper{
    public:
        const FaceData& getData(){
            return dcel->faces[face_index].data;
        }

        EdgeWrapper getEdgeInWrapper() const{
            return new EdgeWrapper(dcel->faces[face_index], dcel);
        }

    private:
        size_t face_index; // current_face_index
        DCEL* dcel;
    };

private:


    struct vertex { //вершины на которых триангулируем
        vertex(Point_2 point, size_t vertex_index, VertexData data) : point(point), outgoing_edge_index(vertex_index), data(data) {};

        Point_2 point; // каждая вершина привязана к точке
        size_t outgoing_edge_index; // индекс ребра в vector<> edges исходящего из этой вершины
        VertexData data;
    };

    struct edge { //ребра, которыми соединяем вершины (называют полу-ребрами). Каждое полу-ребро направлено так, что грань, которой оно принадлежит, находится слева от него.
        edge(size_t face_index, size_t twin_edge_index, size_t prev_edge_index, size_t next_edge_index, EdgeData data)
                : face_index(face_index), twin_edge_index(twin_edge_index), prev_edge_index(prev_edge_index),
                  next_edge_index(next_edge_index), data(data) {};

        size_t face_index; //индекс грани, которая лежит слева от направленного полу-ребра
        size_t twin_edge_index; // индекс ребра, которое может быть (а может и нет) направлено навстречу нашему, при этом соединять те же две вершины, но у него source и target поменяны местами,
        //и соответсвенно face_index у него другой
        size_t prev_edge_index; // индекс грани которая входит в нашу вершину источник (source)
        size_t next_edge_index; // индекс грани которая исходит из нашей вершины сток (target)
        //зная prev и next мы полностью топологически описываем грань, тк грани - треугольники!
        EdgeData data;
    };

    struct face { //грани, собсна треугольники
        face(size_t edge_ind, FaceData data) : any_edge_index(edge_ind), data(data) {};

        size_t any_edge_index; //индекс любого ребра входящего в грань
        FaceData data;
    };


    std::vector<vertex> vertexes;
    std::vector<edge> edges;
    std::vector<face> faces;

};


#endif //HEATEQUATIONWITHDELAUNAYTRIANGULATION_DCEL_H
