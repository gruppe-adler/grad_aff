#pragma once

#include <vector>
#include <memory>

namespace grad_aff {

    class GridBlockTree {
    public:
        GridBlockTree();
        GridBlockTree(bool isLeaf, std::vector<uint8_t> data);
        std::vector<std::shared_ptr<GridBlockTree>> children;
        bool isLeaf;

        std::vector<uint8_t> data;

        void addChild(std::shared_ptr<GridBlockTree> gridBlockTree);
        void removeChild(int index);
    };
};