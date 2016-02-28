#include "include/contexts_factory.h"

#include <stdio.h>
#include <string.h>

#include <queue>
#include <iostream>

ContextsFactory::ContextsFactory(const std::vector<State*>& states,
                                 int n_transitions)
  : states_(states), n_transitions_(n_transitions) {
  const int n_states = states_.size();

  start_contexts_ = new std::vector<int>*[n_states];
  memset(start_contexts_, 0, n_states * sizeof(std::vector<int>*));

  end_contexts_ = new std::vector<int>**[n_states];
  end_context_processed_ = new bool*[n_states];
  for (int i = 0; i < n_states; ++i) {
    const int n_contexts = n_states - i;
    end_context_processed_[i] = new bool[n_contexts];
    memset(end_context_processed_[i], false, n_contexts);

    end_contexts_[i] = new std::vector<int>*[n_contexts];
    memset(end_contexts_[i], 0, n_contexts * sizeof(std::vector<int>*));
  }
}

ContextsFactory::~ContextsFactory() {
  const int n_states = states_.size();
  for (int i = 0; i < n_states; ++i) {
    const int n_contexts = n_states - i;
    for (int j = 0; j < n_contexts; ++j) {
      delete end_contexts_[i][j];
    }
    delete start_contexts_[i];
    delete end_contexts_[i];
    delete end_context_processed_[i];
  }
  delete start_contexts_;
  delete end_contexts_;
  delete end_context_processed_;
}

bool ContextsFactory::FindAnyPath(int from, int to,
                                  std::vector<int>& any_path) const {
  any_path.clear();
  std::vector<bool> visited_states(states_.size(), false);
  visited_states[from] = true;

  std::queue<std::vector<int>* > paths;
  std::queue<State*> states;
  paths.push(new std::vector<int>());
  states.push(states_[from]);

  do {
    std::vector<int>* path = paths.front();
    State* state = states.front();

    if (state->id == to) {
      any_path.resize(path->size());
      std::copy(path->begin(), path->end(), any_path.begin());

      while (!paths.empty()) {
        delete paths.front();
        paths.pop();
      }
      return true;
    }

    for (int i = 0; i < state->transitions_from.size(); ++i) {
      Transition* trans = state->transitions_from[i];
      State* trans_to = trans->to;
      if (!visited_states[trans_to->id]) {
        visited_states[trans_to->id] = true;
        states.push(trans_to);
        std::vector<int>* new_path = new std::vector<int>(*path);
        new_path->push_back(trans->event_id);
        paths.push(new_path);
      }
    }

    delete path;
    paths.pop();
    states.pop();
  } while (!paths.empty());
  return false;
}

void ContextsFactory::InsertBack(std::vector<int>& dst,
                                 const std::vector<int>& src) const {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.end(), src.begin(), src.end());
  }
}

void ContextsFactory::InsertFront(const std::vector<int>& src,
                                  std::vector<int>& dst) const {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.begin(), src.begin(), src.end());
  }
}

void ContextsFactory::FindContextKernels(const std::vector<int>& word,
                                         std::vector<StatesPair>& pairs) const {
  for (int i = 0; i < word.size() && !pairs.empty(); ++i) {
    for (int j = 0; j < pairs.size(); ++j) {
      StatesPair pair = pairs.front();
      pairs.erase(pairs.begin());

      Transition* trans = pair.second->GetTransition(word[i]);
      if (trans != 0) {
        pairs.push_back(StatesPair(pair.first, trans->to));
      }
    }
  }
}

bool ContextsFactory::FindEndContext(State* first_state,
                                     State* second_state,
                                     std::vector<int>& end_context) const {
  end_context.clear();

  int min_id = first_state->id;
  int max_id = second_state->id;
  if (min_id > max_id) std::swap(min_id, max_id);

  if (end_context_processed_[min_id][max_id - min_id]) {
    if (end_contexts_[min_id][max_id - min_id] != 0) {
      InsertFront(*end_contexts_[min_id][max_id - min_id], end_context);
      return true;
    } else {
      return false;
    }
  }

  end_context_processed_[min_id][max_id - min_id] = true;
  end_contexts_[min_id][max_id - min_id] = new std::vector<int>();

  std::queue<std::vector<int>* > contexts;
  std::queue<std::vector<bool>* > used_transitions;
  std::queue<StatesPair> states;

  states.push(StatesPair(first_state, second_state));
  contexts.push(new std::vector<int>());
  used_transitions.push(new std::vector<bool>(n_transitions_, false));

  do {
    std::vector<int>* context = contexts.front();
    std::vector<bool>* transitions = used_transitions.front();
    first_state = states.front().first;
    second_state = states.front().second;

    if (first_state->id != second_state->id) {
      for (int i = 0; i < first_state->transitions_from.size(); ++i) {
        Transition* first_trans = first_state->transitions_from[i];
        const int event_id = first_trans->event_id;
        Transition* second_trans = second_state->GetTransition(event_id);
        if (second_trans != 0 && (!transitions->operator[](first_trans->id) ||
                                  !transitions->operator[](second_trans->id))) {
          std::vector<bool>* new_trans = new std::vector<bool>(*transitions);
          new_trans->operator[](first_trans->id) = true;
          new_trans->operator[](second_trans->id) = true;
          used_transitions.push(new_trans);

          states.push(StatesPair(first_trans->to, second_trans->to));

          std::vector<int>* new_context = new std::vector<int>(*context);
          new_context->push_back(event_id);
          contexts.push(new_context);
        }
      }
    } else {
      bool found = false;
      if (first_state->id != states_.back()->id) {
        found = FindAnyPath(first_state->id, states_.back()->id, end_context);
        InsertFront(end_context, *end_contexts_[min_id][max_id - min_id]);
      } else {
        found = true;
      }
      if (found) {
        InsertFront(*context, end_context);
        InsertFront(end_context, *end_contexts_[min_id][max_id - min_id]);
        while (!contexts.empty()) {
          delete contexts.front();
          contexts.pop();
          delete used_transitions.front();
          used_transitions.pop();
        }
        return true;
      }
    }
    delete context;
    delete transitions;
    contexts.pop();
    used_transitions.pop();
    states.pop();
  } while(!states.empty());
  

  delete end_contexts_[min_id][max_id - min_id];
  end_contexts_[min_id][max_id - min_id] = 0;
  return false;
}

void ContextsFactory::FindStartContext(State* state,
                                       std::vector<int>& start_context) const {
  start_context.clear();
  const int state_id = state->id;
  if (start_contexts_[state_id] != 0) {
    InsertFront(*start_contexts_[state_id], start_context);
  } else {
    start_contexts_[state_id] = new std::vector<int>();
    FindAnyPath(0, state_id, *start_contexts_[state_id]);
    InsertFront(*start_contexts_[state_id], start_context);
  }
}

bool ContextsFactory::FindContext(std::vector<int>& first_substr,
                                  std::vector<int>& second_substr) const {
  const int n_states = states_.size();
  std::vector<StatesPair> first_traces(n_states);
  for (int i = 0; i < n_states; ++i) {
    first_traces[i] = StatesPair(states_[i], states_[i]);
  }

  FindContextKernels(first_substr, first_traces);

  if (first_traces.empty()) return false;

  std::vector<StatesPair> second_traces(first_traces.size());
  for (int i = 0; i < first_traces.size(); ++i) {
    StatesPair pair = first_traces[i];
    pair.second = pair.first;
    second_traces[i] = pair;
  }
  FindContextKernels(second_substr, second_traces);

  if (second_traces.empty()) return false;

  // Find intersections by first state.
  int idx = 0;
  std::vector<int> context;
  for (int i = 0; i < second_traces.size(); ++i) {
    for (int j = idx; j < first_traces.size(); ++j) {
      const int first_id = first_traces[i].first->id;
      const int second_id = second_traces[i].first->id;
      if (first_id == second_id) {
        if (first_traces[i].second->id != n_states - 1 || 
            second_traces[i].second->id != n_states - 1) {
          bool found = FindEndContext(first_traces[i].second,
                                      second_traces[i].second,
                                      context);
          if (found) {
            InsertBack(first_substr, context);
            InsertBack(second_substr, context);

            FindStartContext(first_traces[i].first, context);
            InsertFront(context, first_substr);
            InsertFront(context, second_substr);
            return true;
          }
        } else {
          FindStartContext(first_traces[i].first, context);
          InsertFront(context, first_substr);
          InsertFront(context, second_substr);
          return true;
        }
      }
      ++idx;
    }
  }
  return false;
}

