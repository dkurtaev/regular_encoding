#include "include/code_tree.h"

CodeTree::CodeTree() {
  root_ = new CodeTreeNode();
}

CodeTree::CodeTree(const std::vector<ElementaryCode*>& code) {
  root_ = new CodeTreeNode();
  Add(code);
}

void CodeTree::Add(ElementaryCode* elem_code) {
  CodeTreeNode* current_node = root_;
  for (int i = 0; i < elem_code->str.length(); ++i) {
    if (elem_code->str[i] == '0') {
      if (current_node->GetLeft() == 0) {
        CodeTreeNode* new_node = new CodeTreeNode();
        current_node->SetLeft(new_node);
        current_node = new_node;
      } else {
        current_node = current_node->GetLeft();
      }
    } else {
      if (current_node->GetRight() == 0) {
        CodeTreeNode* new_node = new CodeTreeNode();
        current_node->SetRight(new_node);
        current_node = new_node;
      } else {
        current_node = current_node->GetRight();
      }
    }
  }
  current_node->SetElemCode(elem_code);
}

void CodeTree::Add(const std::vector<ElementaryCode*>& code) {
  for (int i = 0; i < code.size(); ++i) {
    Add(code[i]);
  }
}

CodeTreeNode* CodeTree::Find(const std::string& code) {
  CodeTreeNode* current_node = root_;
  for (int i = 0; i < code.length(); ++i) {
    if (code[i] == '0') {
      if (current_node->GetLeft() != 0) {
        current_node = current_node->GetLeft();
      } else {
        return 0;
      }
    } else {
      if (current_node->GetRight() != 0) {
        current_node = current_node->GetRight();
      } else {
        return 0;
      }
    }
  }
  return current_node;
}

CodeTreeNode* CodeTree::GetRoot() {
  return root_;
}

CodeTree::~CodeTree() {
  delete root_;
}
