#ifndef INCLUDE_CONTEXTS_FACTORY_H_
#define INCLUDE_CONTEXTS_FACTORY_H_

#include <vector>

#include "include/structures.h"

class ContextsFactory {
 public:
  ContextsFactory(const std::vector<State*>& states,
                  int n_transitions);

  ~ContextsFactory();

  bool FindContext(std::vector<int>& first_substr,
                   std::vector<int>& second_substr) const;

 private:
  bool FindAnyPath(int from, int to, std::vector<int>& any_path) const;

  void InsertBack(std::vector<int>& dst, const std::vector<int>& src) const;

  void InsertFront(const std::vector<int>& src, std::vector<int>& dst) const;

  typedef std::pair<State*, State*> StatesPair;
  void FindContextKernels(const std::vector<int>& word,
                          std::vector<StatesPair>& pairs) const;

  bool FindEndContext(State* first_state, State* second_state,
                      std::vector<int>& end_context) const;

  void FindStartContext(State* state, std::vector<int>& start_context) const;

  int n_transitions_;
  std::vector<State*> states_;
  std::vector<int>** start_contexts_;
  std::vector<int>*** end_contexts_;
  bool** end_context_processed_;
};

#endif  // INCLUDE_CONTEXTS_FACTORY_H_