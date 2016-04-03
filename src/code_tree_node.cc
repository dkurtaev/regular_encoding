#include "include/code_tree_node.h"

CodeTreeNode::CodeTreeNode() {
  childs_[1] = childs_[0] = 0;
  elem_code_ = 0;
}

CodeTreeNode::~CodeTreeNode() {
  delete childs_[0];
  delete childs_[1];
}

void CodeTreeNode::Add(CodeTreeNode* root, ElementaryCode* elem_code) {
  const unsigned length = elem_code->str.length();
  for (unsigned i = 0; i < length; ++i) {
    const unsigned char child_id = elem_code->str[i] - '0';
    CodeTreeNode* child = root->childs_[child_id];
    root->lower_elem_codes_.push_back(elem_code);
    if (child) {
      root = child;
    } else {
      root = root->childs_[child_id] = new CodeTreeNode();
    }
  }
  root->elem_code_ = elem_code;
  root->lower_elem_codes_.push_back(elem_code);
}

CodeTreeNode* CodeTreeNode::Find(
    CodeTreeNode* root,
    const std::string& code,
    std::vector<ElementaryCode*>* upper_elem_codes) {
  if (upper_elem_codes) upper_elem_codes->clear();

  const unsigned length = code.length();
  for (unsigned i = 0; i < length; ++i) {
    const unsigned char child_id = code[i] - '0';
    root = root->childs_[child_id];
    if (root) {
      if (upper_elem_codes && root->elem_code_) {
        upper_elem_codes->push_back(root->elem_code_);
      }
    } else {
      return 0;
    }
  }
  return root;
}

void CodeTreeNode::GetLowerElemCodes(
    std::vector<ElementaryCode*>* lower_elem_codes) const {
  lower_elem_codes->clear();
  InsertBack(lower_elem_codes, lower_elem_codes_);
}
