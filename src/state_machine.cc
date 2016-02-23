#include "include/state_machine.h"

#include <fstream>
#include <queue>
#include <iostream>

StateMachine::StateMachine(int n_states) {
  if (n_states != 0) {
    AddStates(n_states);
  }
}

StateMachine::~StateMachine() {
  Clear();
}

void StateMachine::Clear() {
  int n_states = states_.size();
  for (int i = 0; i < n_states; ++i) {
    delete states_[i];
  }
  states_.clear();

  int n_transitions = transitions_.size();
  for (int i = 0; i < n_transitions; ++i) {
    delete transitions_[i];
  }
  transitions_.clear();
}

void StateMachine::AddStates(int n_states) {
  int offset = states_.size();
  for (int i = 0; i < n_states; ++i) {
    states_.push_back(new State(i + offset));
  }
}

void StateMachine::AddTransition(unsigned from_id, unsigned to_id,
                                 unsigned event_id) {
  Transition* trans = new Transition(transitions_.size(),
                                     states_[from_id],
                                     states_[to_id],
                                     event_id);
  transitions_.push_back(trans);
}

void StateMachine::DelTransition(unsigned id) {
  Transition* trans = transitions_[id];
  trans->from->DelTransitionFrom(id);
  trans->to->DelTransitionTo(id);
  delete trans;
  transitions_[id] = 0;
}

void StateMachine::DelState(unsigned id) {
  State* state = states_[id];
  std::vector<int> ids;
  for (int i = 0; i < state->transitions_from.size(); ++i) {
    ids.push_back(state->transitions_from[i]->id);
  }
  for (int i = 0; i < state->transitions_to.size(); ++i) {
    ids.push_back(state->transitions_to[i]->id);
  }
  for (int i = 0; i < ids.size(); ++i) {
    DelTransition(ids[i]);
  }
  delete state;
  states_[id] = 0;
}

State* StateMachine::GetStartState() const {
  return states_[0];
}

State* StateMachine::GetEndState() const {
  return states_.back();
}

State* StateMachine::GetState(int id) const {
  return states_[id];
}

int StateMachine::GetNumberStates() const {
  return states_.size();
}

int StateMachine::GetNumberTransitions() const {
  return transitions_.size();
}

void StateMachine::WriteDot(const std::string& file_path,
                           const std::vector<std::string>& states_names,
                           const std::vector<std::string>& events_names) const {
  std::ofstream file(file_path.c_str());
  file << "strict digraph state_machine {\n";

  const int n_states = states_.size();
  std::string edges[n_states][n_states];
  for (int i = 0; i < n_states; ++i) {
    for (int j = 0; j < n_states; ++j) {
      edges[i][j] = "";
    }
  }

  for (int i = 0; i < n_states; ++i) {
    State* state = states_[i];
    if (state != 0) {
      std::string state_from_name = states_names[i];

      const int n_trans = state->transitions_from.size();
      for (int j = 0; j < n_trans; ++j) {
        Transition* trans = state->transitions_from[j];
        const int state_to_id = trans->to->id;
        std::string state_to_name = states_names[state_to_id];
        std::string event_name = events_names[trans->event_id];

        if (edges[i][state_to_id] != "") {
          edges[i][state_to_id] += ", " + event_name;
        } else {
          edges[i][state_to_id] = event_name;
        }

        file << state_from_name << "->" << state_to_name
             << "[label=\"" <<  edges[i][state_to_id] << "\"];\n";
      }
    }
  }

  file << "}";
  file.close();
}

bool StateMachine::IsRecognized(const std::vector<int>& word) const {
  State* state = states_[0];
  for (int i = 0; i < word.size(); ++i) {
    Transition* trans = state->GetTransition(word[i]);
    if (trans != 0) {
      state = trans->to;
    } else {
      return false;
    }
  }
  return state == states_.back();
}

bool StateMachine::FindSubwords(
                        const std::vector<int>& word,
                        std::vector<StatesPair>& pairs) const {
  for (int i = 0; i < word.size(); ++i) {
    if (!pairs.empty()) {
      const int n_traces = pairs.size();
      for (int j = 0; j < n_traces; ++j) {
        StatesPair trace = pairs.front();
        pairs.erase(pairs.begin());

        Transition* trans = trace.second->GetTransition(word[i]);
        if (trans != 0) {
          pairs.push_back(StatesPair(trace.first, trans->to));
        }
      }
    } else {
      return false;
    }
  }
  return true;
}

bool StateMachine::FindContext(std::vector<int>& first_substr,
                               std::vector<int>& second_substr) const {
  // Track first subword.
  std::vector<StatesPair> first_traces(states_.size());
  for (int i = 0; i < states_.size(); ++i) {
    first_traces[i] = StatesPair(states_[i], states_[i]);
  }
  bool found = FindSubwords(first_substr, first_traces);

  if (!found) return false;

  std::vector<StatesPair> second_traces(first_traces.size());
  for (int i = 0; i < first_traces.size(); ++i) {
    StatesPair pair = first_traces[i];
    pair.second = pair.first;
    second_traces[i] = pair;
  }
  found = FindSubwords(second_substr, second_traces);

  if (!found) return false;

  // Find intersections by first state.
  int idx = 0;
  std::vector<int> context;
  for (int i = 0; i < second_traces.size(); ++i) {
    for (int j = idx; j < first_traces.size(); ++j) {
      if (first_traces[i].first->id == second_traces[i].first->id) {
        bool found = false;

        // Find ways with similar characters from .second states to end state.
        if (first_traces[i].second->id == second_traces[i].second->id) {
          if (first_traces[i].second->id != states_.back()->id) {
            found = FindAnyPath(first_traces[i].second->id, states_.back()->id,
                                context);
          }
        } else {
          found = FindEndOfContext(first_traces[i].second, 
                                   second_traces[i].second, context);
        }


        // Find way from start state to .first
        if (found) {
          InsertBack(first_substr, context);
          InsertBack(second_substr, context);
          if (first_traces[i].first->id != 0) {
            FindAnyPath(0, first_traces[i].first->id, context);
            InsertFront(context, first_substr);
            InsertFront(context, second_substr);
          }
          return true;
        }
      }
      ++idx;
    }
  }
  return false;
}

bool StateMachine::FindAnyPath(int from, int to, std::vector<int>& word) const {
  word.clear();
  std::vector<bool> visited_states(states_.size(), false);
  visited_states[from] = true;

  std::queue<std::vector<int>* > paths;
  std::queue<State*> states;
  paths.push(new std::vector<int>());
  states.push(states_[from]);

  do {
    std::vector<int>* path = paths.front();
    State* state = states.front();

    for (int i = 0; i < state->transitions_from.size(); ++i) {
      Transition* trans = state->transitions_from[i];
      State* trans_to = trans->to;

      if (trans_to->id == to) {
        word.resize(path->size());
        std::copy(path->begin(), path->end(), word.begin());
        word.push_back(trans->event_id);

        while (!paths.empty()) {
          delete paths.front();
          paths.pop();
        }
        return true;
      } else {
        if (!visited_states[trans_to->id]) {
          visited_states[trans_to->id] = true;
          states.push(trans_to);
          std::vector<int>* new_path = new std::vector<int>(*path);
          new_path->push_back(trans->event_id);
          paths.push(new_path);
        }
      }
    }

    delete path;
    paths.pop();
    states.pop();
  } while (!paths.empty());
  return false;
}

bool StateMachine::FindEndOfContext(State* first_state,
                                    State* second_state,
                                    std::vector<int>& end_context) const {
  end_context.clear();
  std::queue<std::vector<int>* > contexts;
  std::queue<std::vector<bool>* > used_transitions;
  std::queue<StatesPair> states;

  states.push(StatesPair(first_state, second_state));
  contexts.push(new std::vector<int>());
  used_transitions.push(new std::vector<bool>(transitions_.size(), false));

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
      } else {
        end_context.resize(context->size());
        std::copy(context->begin(), context->end(), end_context.begin());
        found = true;
      }
      if (found) {
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
  return false;
}

void StateMachine::InsertBack(std::vector<int>& dst,
                              const std::vector<int>& src) const {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.end(), src.begin(), src.end());
  }
}

void StateMachine::InsertFront(const std::vector<int>& src,
                               std::vector<int>& dst) const {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.begin(), src.begin(), src.end());
  }
}
