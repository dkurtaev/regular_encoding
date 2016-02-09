// Copyright 2015-2016 Dmitry Kurtaev

#ifndef INCLUDE_SUFFIX_TREE_H_
#define INCLUDE_SUFFIX_TREE_H_

#include <string>
#include <vector>
#include <iostream>

#include "include/structures.h"

class SuffixTree {
 public:
  SuffixTree();

  void BuildCubic(std::vector<ElementaryCode*>* code);

  void GetSuffixes(std::vector<Suffix*>* suffixes);

  // Return index of suffix in vector from GetSuffixes function.
  // Default value -1 if suffix not found.
  int FindSuffix(const std::string& suffix);

  void Clear();

  void Print(std::ostream* s);

  ~SuffixTree();

 private:
  struct Edge;
  struct Node {
    int id;
    std::vector<Edge*> edges;

    explicit Node(int id = -1);

    ~Node();
  };

  struct Edge {
    Node* to;
    std::string str;

    Edge(Node* to, const std::string& str);
  };

  void ExtendTree(const std::string& prefix, char new_character,
                  ElementaryCode* elem_code);

  // Return 0 if end on node or
  // > 0 if end on edge, where > 0 - index of first different character.
  int FindEnd(const std::string& suffix,
              Node*& end_node,  // NOLINT
              Edge*& end_edge);  // NOLINT

  void PrintNode(Node* node, std::ostream* s, int level);

  Node* root_;
  std::vector<Suffix*> suffixes_;
};

#endif  // INCLUDE_SUFFIX_TREE_H_
