#include "include/code_tree.h"

CodeTree::CodeTree(const std::vector<ElementaryCode*>& code) {
  root_ = new CodeTreeNode();
  Add(code);
}

void CodeTree::Add(const std::vector<ElementaryCode*>& code) {
  const unsigned size = code.size();
  for (unsigned i = 0; i < size; ++i) {
    CodeTreeNode::Add(root_, code[i]);
  }
}

CodeTreeNode* CodeTree::Find(
    const std::string& code,
    std::vector<ElementaryCode*>* upper_elem_codes) const {
  return CodeTreeNode::Find(root_, code, upper_elem_codes);
}

CodeTree::~CodeTree() {
  delete root_;
}
