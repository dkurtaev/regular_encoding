#include "include/simple_suffix_tree.h"

void SimpleSuffixTree::Build(std::vector<ElementaryCode*>* code) {
  int suffix_id = 0;

  // Add root.
  Suffix* empty_suffix = new Suffix(suffix_id++, 0);
  suffixes_.push_back(empty_suffix);
  childs_[0].push_back(-1);
  childs_[1].push_back(-1);

  // For each suffix we try find corresponding node. If node not found, we
  // create it.
  for (int i = 0; i < code->size(); ++i) {
    ElementaryCode* elem_code = (*code)[i];
    elem_code->suffixes.clear();
    std::string word = elem_code->str;
    for (int j = 0; j < word.size(); ++j) {
      int current_vertex = 0;
      for (int k = j; k < word.size(); ++k) {
        int symbol = word[k] - '0';
        if (childs_[symbol][current_vertex] == -1) {
          childs_[symbol][current_vertex] = childs_[symbol].size();
          current_vertex = childs_[symbol].size();
          childs_[symbol].push_back(-1);
          childs_[1 - symbol].push_back(-1);
          suffixes_.push_back(0);
        } else {
          current_vertex = childs_[symbol][current_vertex];
        }
      }
      if (!suffixes_[current_vertex]) {
        suffixes_[current_vertex] = new Suffix(suffix_id++,
                                               word.size() - j,
                                               elem_code);
      } else {
        suffixes_[current_vertex]->owners.push_back(elem_code);
      }
      elem_code->suffixes.push_back(suffixes_[current_vertex]);
    }
  }
  for (int i = 0; i < code->size(); ++i) {
    (*code)[i]->suffixes.push_back(empty_suffix);
    empty_suffix->owners.push_back((*code)[i]);
  }
}

void SimpleSuffixTree::GetSuffixes(std::vector<Suffix*>* suffixes) {
  suffixes->clear();
  for (int i = 0; i < suffixes_.size(); ++i) {
    if (suffixes_[i]) {
      suffixes->push_back(suffixes_[i]);
    }
  }
}

void SimpleSuffixTree::Clear() {
  childs_[0].clear();
  childs_[1].clear();
  for (int i = 0; i < suffixes_.size(); ++i) {
    delete suffixes_[i];
  }
  suffixes_.clear();
}
