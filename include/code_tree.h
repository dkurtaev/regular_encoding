#ifndef INCLUDE_CODE_TREE_H_
#define INCLUDE_CODE_TREE_H_

#include <string>
#include <vector>

#include "include/code_tree_node.h"

class CodeTree {
 public:
  explicit CodeTree(const std::vector<ElementaryCode*>& code);

  ~CodeTree();

  CodeTreeNode* Find(const std::string& code,
                     std::vector<ElementaryCode*>* upper_elem_codes = 0) const;

 private:
  void Add(const std::vector<ElementaryCode*>& code);

  CodeTreeNode* root_;
};

#endif  // INCLUDE_CODE_TREE_H_
