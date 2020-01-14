#include <grad_aff/GridBlockTree.h>

grad_aff::GridBlockTree::GridBlockTree()
{
    this->isLeaf = false;
    this->data = {};
}

grad_aff::GridBlockTree::GridBlockTree(bool isLeaf, std::vector<uint8_t> data)
{
    this->isLeaf = isLeaf;
    this->data = data;
}

void grad_aff::GridBlockTree::addChild(std::shared_ptr<GridBlockTree> gridBlockTree)
{
    this->children.push_back(gridBlockTree);
}
