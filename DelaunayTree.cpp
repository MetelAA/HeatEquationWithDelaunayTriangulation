//
// Created by Artem on 21.05.2026.
//

#include "DelaunayTree.h"

DelaunayTree::location_data DelaunayTree::locate(Point_2 &p) {
    std::unique_ptr<location_data> result;
    const Node* current = nodes[0].get();
    while (current) {
        const Node* next = current->locate(p, result);
        if (next == nullptr) break;   // дошли до листа значит result уже заполнен
        current = next;
    }
    if(result == nullptr)
        throw "face which contains our point is not found!";
    return *result;
}

void DelaunayTree::insert(size_t node_index, DCEL<>::FaceWrapper face) {


    size_t new_node_index = nodes.size();

    nodes[node_index] = Internal(nodes[node_index], node_index, );
}

void DelaunayTree::insert(size_t first_node_index, size_t second_node_index, DCEL<>::FaceWrapper face) {

}
