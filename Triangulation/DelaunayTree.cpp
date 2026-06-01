#include "DelaunayTree.h"

std::vector<size_t> DelaunayTree::insertWhenOneFace(const std::vector<DCEL::FaceWrapper> &newFaces,
                                                    size_t old_leaf_index) {
    std::vector<size_t> newLeafsIndexes = insertNewLeafs(newFaces);
    Internal internal(
        this->nodes_[old_leaf_index]->getV1(),
        this->nodes_[old_leaf_index]->getV2(),
        this->nodes_[old_leaf_index]->getV3(),
        this,
        newLeafsIndexes
    );

    nodes_[old_leaf_index] = std::make_unique<Internal>(internal);
    return newLeafsIndexes;
}

std::vector<size_t> DelaunayTree::insertWhenEdge(const std::vector<DCEL::FaceWrapper> &newFaces,
                                                  size_t old_leaf_index1,
                                                  size_t old_leaf_index2) {
    std::vector<size_t> newLeafsIndexes = insertNewLeafs(newFaces);

    const std::vector<size_t> oldLeafsIndexes = {old_leaf_index1, old_leaf_index2};
    for (const size_t old_leaf_index : oldLeafsIndexes) {
        DCEL::VertexWrapper ov1 = nodes_[old_leaf_index]->getV1();
        DCEL::VertexWrapper ov2 = nodes_[old_leaf_index]->getV2();
        DCEL::VertexWrapper ov3 = nodes_[old_leaf_index]->getV3();

        std::vector<size_t> old_leaf_children;
        for (size_t newLeafIndex : newLeafsIndexes) {
            const Node* child = nodes_[newLeafIndex].get();
            int common = 0;
            if (child->getV1() == ov1 || child->getV1() == ov2 || child->getV1() == ov3) ++common;
            if (child->getV2() == ov1 || child->getV2() == ov2 || child->getV2() == ov3) ++common;
            if (child->getV3() == ov1 || child->getV3() == ov2 || child->getV3() == ov3) ++common;
            if (common == 2) {
                old_leaf_children.push_back(newLeafIndex);
            }
        }

        if(old_leaf_children.size() != 2)
            throw std::runtime_error("Undefined behavior: number of childrens in puncturing with a hit on the rib for both old faces (Leafs) must be equal 2");

        Internal internal(ov1, ov2, ov3, this, old_leaf_children);
        nodes_[old_leaf_index] = std::make_unique<Internal>(internal);
    }

    return newLeafsIndexes;
}

std::vector<size_t> DelaunayTree::insertWhenFlip(const std::vector<DCEL::FaceWrapper> &newFaces,
                                                  size_t old_leaf_index1,
                                                  size_t old_leaf_index2) {
    std::vector<size_t> newLeafsIndexes = insertNewLeafs(newFaces);
    Internal internal1(
        this->nodes_[old_leaf_index1]->getV1(),
        this->nodes_[old_leaf_index1]->getV2(),
        this->nodes_[old_leaf_index1]->getV3(),
        this,
        newLeafsIndexes
    );

    Internal internal2(
        this->nodes_[old_leaf_index2]->getV1(),
        this->nodes_[old_leaf_index2]->getV2(),
        this->nodes_[old_leaf_index2]->getV3(),
        this,
        newLeafsIndexes
    );

    nodes_[old_leaf_index1] = std::make_unique<Internal>(internal1);
    nodes_[old_leaf_index2] = std::make_unique<Internal>(internal2);
    return newLeafsIndexes;
}

std::vector<size_t> DelaunayTree::insertNewLeafs(const std::vector<DCEL::FaceWrapper> &newFaces) {
    std::vector<size_t> newLeafsIndexes; //он же children_indexes
    for (auto &newFace: newFaces) {
        DCEL::VertexWrapper v1 = newFace.getEdge().getSourceVertex();
        DCEL::VertexWrapper v2 = newFace.getEdge().getNextEdge().getSourceVertex();
        DCEL::VertexWrapper v3 = newFace.getEdge().getNextEdge().getNextEdge().getSourceVertex();
        size_t newLeafIndex = nodes_.size();
        newLeafsIndexes.push_back(newLeafIndex);
        nodes_.emplace_back(std::make_unique<Leaf>(v1, v2, v3, this, newFace));
    }
    return newLeafsIndexes;
}
