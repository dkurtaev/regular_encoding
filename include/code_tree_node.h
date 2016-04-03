#ifndef INCLUDE_CODE_TREE_NODE_H_
#define INCLUDE_CODE_TREE_NODE_H_

#include <vector>
#include <iostream>

#include "include/structures.h"

class CodeTreeNode {
 public:
  CodeTreeNode();

  ~CodeTreeNode();

  void SetElemCode(ElementaryCode* elem_code);

  CodeTreeNode* GetChild(unsigned char id) const;

  void SetChild(unsigned char id, CodeTreeNode* child);

//  void GetUpperElemCodes(
//      std::vector<const ElementaryCode*>* upper_elem_codes) const;

//  void GetLowerElemCodes(
//      std::vector<const ElementaryCode*>* lower_elem_codes) const;

  void Print() {
    if (childs_[0]) childs_[0]->Print();
    if (childs_[1]) childs_[1]->Print();
    std::cout << lower_elem_codes_from_ << ' ' << lower_elem_codes_to_ << std::endl;

  }

  // Returns ids of elementary codes in depth bypass order.
  // Argument [permutation] must has size equals number of elementary codes.
  void SetupLowerElemCodesBorders(std::vector<int>* permutation);

 private:
  int lower_elem_codes_from_;
  int lower_elem_codes_to_;
  int upper_elem_codes_from_;
  int upper_elem_codes_to_;

  ElementaryCode* elem_code_;
  CodeTreeNode* childs_[2];
};

#endif  // INCLUDE_CODE_TREE_NODE_H_
