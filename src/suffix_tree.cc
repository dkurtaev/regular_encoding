// Copyright 2015-2016 Dmitry Kurtaev

#include "include/suffix_tree.h"

SuffixTree::SuffixTree()
  : root_(0) {
}

void SuffixTree::BuildCubic(std::vector<ElementaryCode*>* code) {
  Clear();

  Suffix* empty_suffix = new Suffix(suffixes_.size(), 0);
  suffixes_.push_back(empty_suffix);

  root_ = new Node();
  root_->edges.push_back(new Edge(new Node(), "."));

  for (int i = 0; i < code->size(); ++i) {
    ElementaryCode* elem_code = code->at(i);
    elem_code->suffixes.clear();
    std::string word = elem_code->str + '.';
    bool need_new_edge = true;

    // Find edge begins first character.
    for (int j = 0; j < root_->edges.size(); ++j) {
      Edge* edge = root_->edges[j];
      if (edge->str[0] == word[0]) {
        need_new_edge = false;
        break;
      }
    }
    if (need_new_edge) {
      root_->edges.push_back(new Edge(new Node(),
                                      std::string(1, word[0])));
    }
    for (int j = 1; j < word.length(); ++j) {
      ExtendTree(word.substr(0, j), word[j], elem_code);
    }
  }
  for (int i = 0; i < code->size(); ++i) {
    code->at(i)->suffixes.push_back(empty_suffix);
    empty_suffix->owners.push_back(code->at(i));
  }
}

void SuffixTree::ExtendTree(const std::string &prefix, char new_character,
                            ElementaryCode* elem_code) {
  for (int i = 0; i <= prefix.length(); ++i) {
    std::string suffix = prefix.substr(i);
    Edge* end_edge;
    Node* end_node;
    int split_index = FindEnd(suffix, end_node, end_edge);

    if (split_index == 0) {
      // If end of suffix is leaf
      if (end_node->edges.size() == 0) {
        end_edge->str += new_character;
        if (new_character == '.') {
          end_node->id = suffixes_.size();
          Suffix* new_suffix = new Suffix(suffixes_.size(),
                                          suffix.length(),
                                          elem_code);
          suffixes_.push_back(new_suffix);
          elem_code->suffixes.push_back(new_suffix);
        }
      } else {
        // If end of suffix is internal node.
        bool need_new_edge = true;

        // Find edge begins new character.
        for (int j = 0; j < end_node->edges.size(); ++j) {
          Edge* edge = end_node->edges[j];
          if (edge->str[0] == new_character) {
            need_new_edge = false;
            if (suffix != "" && new_character == '.') {
              elem_code->suffixes.push_back(suffixes_[edge->to->id]);
              suffixes_[edge->to->id]->owners.push_back(elem_code);
            }
            break;
          }
        }

        if (need_new_edge) {
          Node* new_node = new Node();
          end_node->edges.push_back(new Edge(new_node,
                                             std::string(1, new_character)));
          if (new_character == '.') {
            new_node->id = suffixes_.size();
            Suffix* new_suffix = new Suffix(suffixes_.size(),
                                            suffix.length(),
                                            elem_code);
            suffixes_.push_back(new_suffix);
            elem_code->suffixes.push_back(new_suffix);
          }
        }
      }
    } else {
      // If end of suffix on edge
      if (end_edge->str[split_index] != new_character) {
        Node* delimeter = new Node();
        Node* new_node = new Node();
        delimeter->edges.push_back(new Edge(end_edge->to,
                                            end_edge->str.substr(split_index)));
        delimeter->edges.push_back(new Edge(new_node,
                                            std::string(1, new_character)));
        end_edge->str = end_edge->str.substr(0, split_index);
        end_edge->to = delimeter;
        if (new_character == '.') {
          new_node->id = suffixes_.size();
          Suffix* new_suffix = new Suffix(suffixes_.size(),
                                          suffix.length(),
                                          elem_code);
          suffixes_.push_back(new_suffix);
          elem_code->suffixes.push_back(new_suffix);
        }
      } else {
        if (new_character == '.') {
          elem_code->suffixes.push_back(suffixes_[end_edge->to->id]);
          suffixes_[end_edge->to->id]->owners.push_back(elem_code);
        }
      }
    }
  }
}

int SuffixTree::FindEnd(const std::string &suffix,
                        Node*& end_node,
                        Edge*& end_edge) {
  end_edge = 0;
  end_node = root_;
  for (int i = 0; i < suffix.length();) {
    // Find edge begins suffix's character.
    for (int j = 0; j < end_node->edges.size(); ++j) {
      Edge* edge = end_node->edges[j];
      if (edge->str[0] == suffix[i]) {
        end_edge = edge;
        end_node = 0;
        break;
      }
    }

    // If founded.
    if (end_edge != 0) {
      ++i;
      int k;
      for (k = 1; k < end_edge->str.length(); ++k) {
        if (i < suffix.length() &&
            end_edge->str[k] == suffix[i]) {
          ++i;
        } else {
          return k;
        }
      }
      end_node = end_edge->to;
    } else {
      return 0;
    }
  }
  return 0;
}

void SuffixTree::Clear() {
  delete root_;
  root_ = 0;
  for (int i = 0; i < suffixes_.size(); ++i) {
    delete suffixes_[i];
  }
  suffixes_.clear();
}

void SuffixTree::GetSuffixes(std::vector<Suffix*>* suffixes) {
  suffixes->clear();
  for (int i = 0; i < suffixes_.size(); ++i) {
    suffixes->push_back(suffixes_[i]);
  }
}

int SuffixTree::FindSuffix(const std::string& suffix) {
  Node* end_node;
  Edge* end_edge;
  int split_idx = FindEnd(suffix + '.', end_node, end_edge);
  if (split_idx == 0) {
    return end_node->id;
  } else {
    return -1;
  }
}

void SuffixTree::Print(std::ostream* s) {
  PrintNode(root_, s, 0);
}

void SuffixTree::PrintNode(Node* node,
                           std::ostream* s, int level) {
  for (int i = 0; i < node->edges.size(); ++i) {
    for (int j = 0; j < level; ++j) {
      *s << "    ";
    }
    *s << "|-- " << node->edges[i]->str << '\n';
    PrintNode(node->edges[i]->to, s, level + 1);
  }
}

SuffixTree::Node::~Node() {
  for (int i = 0; i < edges.size(); ++i) {
    delete edges[i]->to;
    delete edges[i];
  }
  edges.clear();
}

SuffixTree::Edge::Edge(Node* to, const std::string& str)
  : to(to),
    str(str) {
}

SuffixTree::~SuffixTree() {
  delete root_;
  suffixes_.clear();
}

SuffixTree::Node::Node(int id)
  : id(id) {
}
