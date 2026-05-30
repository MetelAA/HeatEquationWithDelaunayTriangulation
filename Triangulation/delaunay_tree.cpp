#include "delaunay_tree.h"

std::vector<size_t> delaunay_tree::insert(std::vector<dcel::FaceWrapper> &newFaces,
                                          std::vector<size_t> &old_leafs_indexes) {
    std::vector<size_t> newLeafsIndexes; //он же children_indexes

    for (int i = 0; i < newFaces.size(); ++i) {
        dcel::VertexWrapper v1 = newFaces[i].getEdge().getSourceVertex();
        dcel::VertexWrapper v2 = newFaces[i].getEdge().getNextEdge().getSourceVertex();
        dcel::VertexWrapper v3 = newFaces[i].getEdge().getNextEdge().getNextEdge().getSourceVertex();
        size_t newLeafIndex = nodes_.size();
        newLeafsIndexes.push_back(newLeafIndex);
        nodes_.emplace_back(std::make_unique<Leaf>(v1, v2, v3, this, newFaces[i]));
    }


    if (newFaces.size() == 4 && old_leafs_indexes.size() == 2) {
        //значит попали на ребро
        //у нас есть все дети, в данном случае 4, в детях есть точки (vertex) мы можем однозначно по наличию двух точек определить на терретории какой из предыдущих face ледит эта node (face которая закрепелена за node)

        for (int i = 0; i < old_leafs_indexes.size(); ++i) {

            const Node* oldNode = nodes_[old_leafs_indexes[i]].get();

            std::vector<size_t> curOldNodeChildren;
            const dcel::VertexWrapper& ov1 = oldNode->getV1();
            const dcel::VertexWrapper& ov2 = oldNode->getV2();
            const dcel::VertexWrapper& ov3 = oldNode->getV3();

            for (size_t child_idx : newLeafsIndexes) {
                const Node* child = nodes_[child_idx].get();
                int common = 0;
                if (child->getV1() == ov1 || child->getV1() == ov2 || child->getV1() == ov3) ++common;
                if (child->getV2() == ov1 || child->getV2() == ov2 || child->getV2() == ov3) ++common;
                if (child->getV3() == ov1 || child->getV3() == ov2 || child->getV3() == ov3) ++common;
                if (common == 2) {
                    curOldNodeChildren.push_back(child_idx);
                }
            }

            if(curOldNodeChildren.size() != 2)
                throw std::runtime_error("Undefined behavior: number of childrens in puncturing with a hit on the rib for both old faces (Leafs) must be equal 2");

            Internal internal(ov1, ov2, ov3, this, curOldNodeChildren);
            nodes_[old_leafs_indexes[i]] = std::make_unique<Internal>(internal);
        }
        return newLeafsIndexes;
    } else if (newFaces.size() == 3 && old_leafs_indexes.size() == 1) {
        //попали внутрь грани


        Internal internal(this->nodes_[old_leafs_indexes[0]]->getV1(),
                          this->nodes_[old_leafs_indexes[0]]->getV2(),
                          this->nodes_[old_leafs_indexes[0]]->getV3(),
                          this,
                          newLeafsIndexes
        );
        nodes_[old_leafs_indexes[0]] = std::make_unique<Internal>(internal);
        return newLeafsIndexes;
    } else if (newFaces.size() == 2 && old_leafs_indexes.size() == 2) {
        //случай после флипа ребер
        Internal internal1(
                this->nodes_[old_leafs_indexes[0]]->getV1(),
                this->nodes_[old_leafs_indexes[0]]->getV2(),
                this->nodes_[old_leafs_indexes[0]]->getV3(),
                this,
                newLeafsIndexes
                );
        Internal internal2(
                this->nodes_[old_leafs_indexes[1]]->getV1(),
                this->nodes_[old_leafs_indexes[1]]->getV2(),
                this->nodes_[old_leafs_indexes[1]]->getV3(),
                this,
                newLeafsIndexes
        );

        nodes_[old_leafs_indexes[0]] = std::make_unique<Internal>(internal1);
        nodes_[old_leafs_indexes[1]] = std::make_unique<Internal>(internal2);
        return newLeafsIndexes;
    } else {
        throw std::runtime_error("undefined behavior, error in filling vectors");
    }
}
