#ifndef INCLUDE_CODE_TREE_NODE_H_
#define INCLUDE_CODE_TREE_NODE_H_

#include <vector>

#include "include/structures.h"

class CodeTreeNode {
 public:
  CodeTreeNode(unsigned char id, CodeTreeNode* parent);

  void SetElemCode(ElementaryCode* elem_code);

  CodeTreeNode* GetChild(unsigned char id) const;

  void GetUpperElemCodes(
      std::vector<const ElementaryCode*>* upper_elem_codes) const;

  void GetLowerElemCodes(
      std::vector<const ElementaryCode*>* lower_elem_codes) const;

  ~CodeTreeNode();

 private:
  // Includes current elm.code.
  std::vector<const ElementaryCode*> upper_elem_codes_;
  // Includes current elm.code.
  std::vector<const ElementaryCode*> lower_elem_codes_;
  std::vector<CodeTreeNode*> upper_nodes_;
  std::vector<CodeTreeNode*> lower_nodes_;
  CodeTreeNode* childs_[2];
};

#endif  // INCLUDE_CODE_TREE_NODE_H_
