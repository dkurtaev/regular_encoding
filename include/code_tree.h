#ifndef INCLUDE_CODE_TREE_H_
#define INCLUDE_CODE_TREE_H_

#include <string>
#include <vector>

#include "include/code_tree_node.h"

class CodeTree {
 public:
  CodeTree();

  explicit CodeTree(const std::vector<ElementaryCode*>& code);

  ~CodeTree();

  void Add(ElementaryCode* elem_code);

  void Add(const std::vector<ElementaryCode*>& code);

  bool Find(const std::string& code, CodeTreeNode*& last_node) const;

 private:
  CodeTreeNode* root_;
};

#endif  // INCLUDE_CODE_TREE_H_
