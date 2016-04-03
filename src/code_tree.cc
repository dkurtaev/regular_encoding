#include "include/code_tree.h"

CodeTree::CodeTree(const std::vector<ElementaryCode*>& code,
                   std::vector<int>* permutation) {
  root_ = new CodeTreeNode();
  Add(code, permutation);
}

void CodeTree::Add(ElementaryCode* elem_code) {
  CodeTreeNode* current_node = root_;
  const unsigned length = elem_code->str.length();
  for (unsigned i = 0; i < length; ++i) {
    const unsigned char child_id = elem_code->str[i] - '0';
    CodeTreeNode* child = current_node->GetChild(child_id);
    if (child) {
      current_node = child;
    } else {
      child = new CodeTreeNode();
      current_node->SetChild(child_id, child);
      current_node = child;
    }
  }
  current_node->SetElemCode(elem_code);
}

void CodeTree::Add(const std::vector<ElementaryCode*>& code,
                   std::vector<int>* permutation) {
  for (int i = 0; i < code.size(); ++i) {
    Add(code[i]);
  }
  permutation->resize(code.size());
  root_->SetupLowerElemCodesBorders(permutation);
  root_->Print();
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
    } else {
      return false;
    }
  }
  return true;
}

CodeTree::~CodeTree() {
  delete root_;
}
