#include "DelaunayTree.h"

std::vector<size_t> DelaunayTree::insert(DCEL::VertexWrapper p_r, std::vector<DCEL::FaceWrapper> &newFaces,
                                         std::vector<size_t> &old_leafs_indexes) {
    std::vector<size_t> newLeafsIndexes;
    std::vector<Node const *> children;

    for (int i = 0; i < newFaces.size(); ++i) {
        DCEL::VertexWrapper v1 = newFaces[i].getEdge().getSourceVertex();
        DCEL::VertexWrapper v2 = newFaces[i].getEdge().getNextEdge().getSourceVertex();
        DCEL::VertexWrapper v3 = newFaces[i].getEdge().getNextEdge().getNextEdge().getSourceVertex();
        size_t newLeafIndex = nodes_.size();
        newLeafsIndexes.push_back(newLeafIndex);
        nodes_.emplace_back(std::make_unique<Leaf>(Leaf(v1, v2, v3, newFaces[i])));
        children.push_back(nodes_[newLeafIndex].get());
    }

    if (newFaces.size() == 4 && old_leafs_indexes.size() == 2) {
        //значит попали на ребро
        //у нас есть все дети, в данном случае 4, в детях есть точки (vertex) мы можем однозначно по наличию двух точек определить на терретории какой из предыдущих face ледит эта node (face которая закрепелена за node)

        for (int i = 0; i < old_leafs_indexes.size(); ++i) {

            const auto& oldLeaf = static_cast<const Leaf&>(*nodes_[old_leafs_indexes[i]]);

            std::vector<const Node*> curOldNodeChilderen;
            const DCEL::VertexWrapper& ov1 = oldLeaf.getV1();
            const DCEL::VertexWrapper& ov2 = oldLeaf.getV2();
            const DCEL::VertexWrapper& ov3 = oldLeaf.getV3();

            for (int j = 0; j < children.size(); ++j) {
                int common = 0;
                if (children[j]->getV1() == ov1 || children[j]->getV1() == ov2 || children[j]->getV1() == ov3) ++common;
                if (children[j]->getV2() == ov1 || children[j]->getV2() == ov2 || children[j]->getV2() == ov3) ++common;
                if (children[j]->getV3() == ov1 || children[j]->getV3() == ov2 || children[j]->getV3() == ov3) ++common;
                if (common == 2) {
                    curOldNodeChilderen.push_back(children[j]);
                }
            }
            if(curOldNodeChilderen.size() != 2)
                throw std::runtime_error("Undefined behavior: number of childrens in puncturing with a hit on the rib for both old faces (Leafs) must be equal 2");

            Internal internal(ov1, ov2, ov3, curOldNodeChilderen);
            nodes_[old_leafs_indexes[0]] = std::make_unique<Internal>(internal);
        }
        return newLeafsIndexes;
    } else if (newFaces.size() == 3 && old_leafs_indexes.size() == 1) {
        //попали внутрь грани


        Internal internal(this->nodes_[old_leafs_indexes[0]]->getV1(),
                          this->nodes_[old_leafs_indexes[0]]->getV2(),
                          this->nodes_[old_leafs_indexes[0]]->getV3(),
                          children
        );
        nodes_[old_leafs_indexes[0]] = std::make_unique<Internal>(internal);
        return newLeafsIndexes;
    } else if (newFaces.size() == 2 && old_leafs_indexes.size() == 2) {
        //случай после флипа ребер
        Internal internal1(
                this->nodes_[old_leafs_indexes[0]]->getV1(),
                this->nodes_[old_leafs_indexes[0]]->getV2(),
                this->nodes_[old_leafs_indexes[0]]->getV3(),
                children
                );
        Internal internal2(
                this->nodes_[old_leafs_indexes[1]]->getV1(),
                this->nodes_[old_leafs_indexes[1]]->getV2(),
                this->nodes_[old_leafs_indexes[1]]->getV3(),
                children
        );

        nodes_[old_leafs_indexes[0]] = std::make_unique<Internal>(internal1);
        nodes_[old_leafs_indexes[1]] = std::make_unique<Internal>(internal2);
        return newLeafsIndexes;
    } else {
        throw std::runtime_error("undefined behavior, error in filling vectors");
    }
}
