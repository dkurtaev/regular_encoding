#include "include/code_tree.h"

CodeTree::CodeTree() {
  root_ = new CodeTreeNode(0, 0);
}

CodeTree::CodeTree(const std::vector<ElementaryCode*>& code) {
  root_ = new CodeTreeNode(0, 0);
  Add(code);
}

void CodeTree::Add(ElementaryCode* elem_code) {
  CodeTreeNode* current_node = root_;
  for (int i = 0; i < elem_code->str.length(); ++i) {
    const unsigned char child_id = elem_code->str[i] - '0';
    CodeTreeNode* child = current_node->GetChild(child_id);
    if (child) {
      current_node = child;
    } else {
      current_node = new CodeTreeNode(child_id, current_node);
    }
  }
  current_node->SetElemCode(elem_code);
}

void CodeTree::Add(const std::vector<ElementaryCode*>& code) {
  for (int i = 0; i < code.size(); ++i) {
    Add(code[i]);
  }
}

bool CodeTree::Find(const std::string& code, CodeTreeNode*& last_node) const {
  last_node = root_;
  CodeTreeNode* child = 0;
  const unsigned length = code.length();
  for (unsigned i = 0; i < length; ++i) {
    const unsigned char child_id = code[i] - '0';
    child = last_node->GetChild(child_id);
    if (child) {
      last_node = child;
    }
    else {
      return false;
    }
  }
  return true;
}

CodeTree::~CodeTree() {
  delete root_;
}
