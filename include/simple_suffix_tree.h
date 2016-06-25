// Copyright © 2016 Dmitry Kurtaev. All rights reserved.
// License: MIT License (see LICENSE)
// e-mail: dmitry.kurtaev@gmail.com

#ifndef INCLUDE_SIMPLE_SUFFIX_TREE_H_
#define INCLUDE_SIMPLE_SUFFIX_TREE_H_

#include <vector>

#include "include/structures.h"

class SimpleSuffixTree {
 public:
  void Build(std::vector<ElementaryCode*>* code);

  void GetSuffixes(std::vector<Suffix*>* suffixes);

  void Clear();

 private:
  // Index to vertex after corresponding symbol.
  std::vector<int> childs_[2];
  // Suffixes contained in vertices. 0 if simple node.
  std::vector<Suffix*> vertices_content_;
  // All suffixes.
  std::vector<Suffix*> suffixes_;
};

#endif  // INCLUDE_SIMPLE_SUFFIX_TREE_H_
