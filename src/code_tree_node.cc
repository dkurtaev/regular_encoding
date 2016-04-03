#include "include/code_tree_node.h"

CodeTreeNode::CodeTreeNode() {
  childs_[1] = childs_[0] = 0;
  lower_elem_codes_from_ = -1;
  lower_elem_codes_to_ = -2;
  elem_code_ = 0;
}

CodeTreeNode::~CodeTreeNode() {
  delete childs_[0];
  delete childs_[1];
}

void CodeTreeNode::SetupLowerElemCodesBorders(CodeTreeNode* root,
                                              std::vector<int>* permutation) {
  unsigned idx = 0;
  std::vector<CodeTreeNode*> childs_stack;
  childs_stack.push_back(root);

  CodeTreeNode* node;
  CodeTreeNode* child;
  bool childs_is_processed;

  do {
    node = childs_stack.back();

    childs_is_processed = true;
    for (int i = 1; i >= 0; --i) {
      child = node->childs_[i];
      if (child) {
        if (child->lower_elem_codes_from_ < 0) {
          childs_stack.push_back(child);
          childs_is_processed = false;
        }
      }
    }

    if (childs_is_processed) {
      if (node->childs_[0]) {
        node->lower_elem_codes_from_ =node->childs_[0]
                                      ->lower_elem_codes_from_;
      } else {
        if (node->childs_[1]) {
          node->lower_elem_codes_from_ = node->childs_[1]
                                         ->lower_elem_codes_from_;
        } else {
          node->lower_elem_codes_from_ = idx;
        }
      }

      if (node->childs_[1]) {
        node->lower_elem_codes_to_ = node->childs_[1]->lower_elem_codes_to_;
      } else {
        if (node->childs_[0]) {
          node->lower_elem_codes_to_ = node->childs_[0]->lower_elem_codes_to_;
        } else {
          node->lower_elem_codes_to_ = idx;
        }
      }

      if (node->elem_code_) {
        permutation->operator [](idx++) = node->elem_code_->id;
        if (node->childs_[0] || node->childs_[1]) {
          ++node->lower_elem_codes_to_;
        }
      }
      childs_stack.pop_back();
    }
  } while (!childs_stack.empty());
}

void CodeTreeNode::Add(CodeTreeNode* root, ElementaryCode* elem_code) {
  const unsigned length = elem_code->str.length();
  for (unsigned i = 0; i < length; ++i) {
    const unsigned char child_id = elem_code->str[i] - '0';
    CodeTreeNode* child = root->childs_[child_id];
    if (child) {
      root = child;
    } else {
      root = root->childs_[child_id] = new CodeTreeNode();
    }
  }
  root->elem_code_ = elem_code;
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

void CodeTreeNode::GetLowerElemCodesRange(int* from, int* to) const {
  *from = lower_elem_codes_from_;
  *to = lower_elem_codes_to_;
}
