#include "b_plus_tree_node.h"

BPlusTreeNode::BPlusTreeNode(bool leaf)
    : isLeaf(leaf),
      next(nullptr)
{
}
