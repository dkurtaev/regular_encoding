#include "include/code_tree_node.h"

#include <iostream>

CodeTreeNode::CodeTreeNode() {
  childs_[1] = childs_[0] = 0;
  lower_elem_codes_from_ = lower_elem_codes_to_ =
      upper_elem_codes_from_ = upper_elem_codes_to_ = -1;
  elem_code_ = 0;
}

CodeTreeNode* CodeTreeNode::GetChild(unsigned char id) const {
  return childs_[id];
}

void CodeTreeNode::SetChild(unsigned char id, CodeTreeNode* child) {
  childs_[id] = child;
}

//void CodeTreeNode::GetUpperElemCodes(
//      std::vector<const ElementaryCode*>* upper_elem_codes) const {
//  upper_elem_codes->clear();
//  InsertFront(upper_elem_codes_, upper_elem_codes);
//}

//void CodeTreeNode::GetLowerElemCodes(
//      std::vector<const ElementaryCode*>* lower_elem_codes) const {
//  lower_elem_codes->clear();
//  InsertFront(lower_elem_codes_, lower_elem_codes);
//}

void CodeTreeNode::SetElemCode(ElementaryCode* elem_code) {
  elem_code_ = elem_code;
}

CodeTreeNode::~CodeTreeNode() {
  delete childs_[0];
  delete childs_[1];
}

void CodeTreeNode::SetupLowerElemCodesBorders(std::vector<int>* permutation) {
  unsigned idx = 0;
  std::vector<CodeTreeNode*> childs_stack;
  childs_stack.push_back(this);

  CodeTreeNode* node;
  CodeTreeNode* child;
  bool childs_is_processed;

  do {
    node = childs_stack.back();

    childs_is_processed = true;
    for (int i = 1; i >= 0; --i) {
      child = node->childs_[i];
      if (child) {
        if (child->lower_elem_codes_from_ == -1) {
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
