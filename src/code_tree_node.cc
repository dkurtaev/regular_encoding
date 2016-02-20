#include "include/code_tree_node.h"

CodeTreeNode::CodeTreeNode(CodeTreeNode* left,
                           CodeTreeNode* right,
                           ElementaryCode* elem_code)
  : left_(left),
    right_(right),
    elem_code_(elem_code) {
}


CodeTreeNode* CodeTreeNode::GetLeft() const {
  return left_;
}

CodeTreeNode* CodeTreeNode::GetRight() const {
  return right_;
}

ElementaryCode* CodeTreeNode::GetElemCode() const {
  return elem_code_;
}

void CodeTreeNode::SetLeft(CodeTreeNode* left) {
  delete left_;
  left_ = left;
}

void CodeTreeNode::SetRight(CodeTreeNode* right) {
  delete right_;
  right_ = right;
}

void CodeTreeNode::SetElemCode(ElementaryCode* elem_code) {
  elem_code_ = elem_code;
}

CodeTreeNode::~CodeTreeNode() {
  delete left_;
  left_ = 0;
  delete right_;
  right_ = 0;
}
