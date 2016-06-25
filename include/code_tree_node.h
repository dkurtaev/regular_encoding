// Copyright © 2016 Dmitry Kurtaev. All rights reserved.
// License: MIT License (see LICENSE)
// e-mail: dmitry.kurtaev@gmail.com

#ifndef INCLUDE_CODE_TREE_NODE_H_
#define INCLUDE_CODE_TREE_NODE_H_

#include <vector>
#include <string>

#include "include/structures.h"

class CodeTreeNode {
 public:
  CodeTreeNode();

  ~CodeTreeNode();

  static void Add(CodeTreeNode* root, ElementaryCode* elem_code);

  static CodeTreeNode* Find(CodeTreeNode* root,
                            const std::string& code,
                            std::vector<ElementaryCode*>* upper_elem_codes = 0);

  void GetLowerElemCodes(std::vector<ElementaryCode*>* lower_elem_codes) const;

 private:
  std::vector<ElementaryCode*> lower_elem_codes_;
  ElementaryCode* elem_code_;
  CodeTreeNode* childs_[2];
};

#endif  // INCLUDE_CODE_TREE_NODE_H_
