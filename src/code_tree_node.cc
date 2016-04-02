#include "include/code_tree_node.h"

#include <iostream>

CodeTreeNode::CodeTreeNode(unsigned char id, CodeTreeNode* parent) {
  childs_[1] = childs_[0] = 0;
  if (parent) {
    InsertFront(parent->upper_nodes_, &upper_nodes_);
    upper_nodes_.reserve(upper_nodes_.size() + 1);
    upper_nodes_.insert(upper_nodes_.end(), parent);

    if (parent->childs_[id]) {
      std::cout << "Tree structure is broken" << std::endl;
    }
    parent->childs_[id] = this;

    std::vector<CodeTreeNode*>* vect = 0;
    const unsigned size = upper_nodes_.size();
    for (unsigned i = 0; i < size; ++i) {
      vect = &upper_nodes_[i]->lower_nodes_;
      vect->reserve(vect->size() + 1);
      vect->insert(vect->end(), this);
    }

    if (!parent->upper_elem_codes_.empty()) {
      InsertFront(parent->upper_elem_codes_, &upper_elem_codes_);
    }
  }
}

CodeTreeNode* CodeTreeNode::GetChild(unsigned char id) const {
  return childs_[id];
}

void CodeTreeNode::GetUpperElemCodes(
      std::vector<const ElementaryCode*>* upper_elem_codes) const {
  upper_elem_codes->clear();
  InsertFront(upper_elem_codes_, upper_elem_codes);
}

void CodeTreeNode::GetLowerElemCodes(
      std::vector<const ElementaryCode*>* lower_elem_codes) const {
  lower_elem_codes->clear();
  InsertFront(lower_elem_codes_, lower_elem_codes);
}

void CodeTreeNode::SetElemCode(ElementaryCode* elem_code) {
  upper_elem_codes_.reserve(upper_elem_codes_.size() + 1);
  upper_elem_codes_.insert(upper_elem_codes_.end(), elem_code);
  lower_elem_codes_.reserve(lower_elem_codes_.size() + 1);
  lower_elem_codes_.insert(lower_elem_codes_.end(), elem_code);

  std::vector<const ElementaryCode*>* vect = 0;
  unsigned size = upper_nodes_.size();
  for (unsigned i = 0; i < size; ++i) {
    vect = &upper_nodes_[i]->lower_elem_codes_;
    vect->reserve(vect->size() + 1);
    vect->insert(vect->end(), elem_code);
  }

  size = lower_nodes_.size();
  for (unsigned i = 0; i < size; ++i) {
    vect = &lower_nodes_[i]->upper_elem_codes_;
    vect->reserve(vect->size() + 1);
    vect->insert(vect->end(), elem_code);
  }
}

CodeTreeNode::~CodeTreeNode() {
  delete childs_[0];
  delete childs_[1];
}
