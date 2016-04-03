#ifndef INCLUDE_CODE_TREE_NODE_H_
#define INCLUDE_CODE_TREE_NODE_H_

#include <vector>
#include <iostream>

#include "include/structures.h"

class CodeTreeNode {
 public:
  CodeTreeNode();

  ~CodeTreeNode();

  // Returns ids of elementary codes in depth bypass order.
  // Argument [permutation] must has size equals number of elementary codes.
  static void SetupLowerElemCodesBorders(CodeTreeNode* root,
                                         std::vector<int>* permutation);

  static void SetupUpperElemCodesBorders(ElementaryCode* root);

  static void Add(CodeTreeNode* root, ElementaryCode* elem_code);

  static CodeTreeNode* Find(CodeTreeNode* root,
                            const std::string& code,
                            std::vector<ElementaryCode*>* upper_elem_codes = 0);

  void GetLowerElemCodesRange(int* from, int* to) const;

 private:
  int lower_elem_codes_from_;
  int lower_elem_codes_to_;

  ElementaryCode* elem_code_;
  CodeTreeNode* childs_[2];
};

#endif  // INCLUDE_CODE_TREE_NODE_H_
