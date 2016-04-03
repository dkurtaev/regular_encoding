#ifndef INCLUDE_CODE_TREE_H_
#define INCLUDE_CODE_TREE_H_

#include <string>
#include <vector>

#include "include/code_tree_node.h"

class CodeTree {
 public:
  // Returns ids of elementary codes in depth bypass order.
  CodeTree(const std::vector<ElementaryCode*>& code,
           std::vector<int>* permutation);

  ~CodeTree();

  bool Find(const std::string& code, CodeTreeNode*& last_node) const;

 private:
  void Add(ElementaryCode* elem_code);

  void Add(const std::vector<ElementaryCode*>& code,
           std::vector<int>* permutation);

  CodeTreeNode* root_;
};

#endif  // INCLUDE_CODE_TREE_H_
