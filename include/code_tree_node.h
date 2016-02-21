#ifndef INCLUDE_CODE_TREE_NODE_H_
#define INCLUDE_CODE_TREE_NODE_H_

#include "include/structures.h"

class CodeTreeNode {
 public:
  CodeTreeNode(CodeTreeNode* left = 0,
               CodeTreeNode* right = 0,
               ElementaryCode* elem_code = 0);

  void SetLeft(CodeTreeNode* left);

  void SetRight(CodeTreeNode* right);

  void SetElemCode(ElementaryCode* elem_code);

  CodeTreeNode* GetLeft() const;

  CodeTreeNode* GetRight() const;

  ElementaryCode* GetElemCode() const;

  ~CodeTreeNode();

 private:
  ElementaryCode* elem_code_;
  CodeTreeNode* left_;
  CodeTreeNode* right_;
};

#endif  // INCLUDE_CODE_TREE_NODE_H_
