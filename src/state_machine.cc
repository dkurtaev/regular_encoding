#include "include/state_machine.h"

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <queue>
#include <iostream>

StateMachine::StateMachine(int n_states) {
  if (n_states != 0) {
    Init(n_states);
  }
}

StateMachine::~StateMachine() {
  Clear();
}

void StateMachine::Init(int n_states) {
  if (!states_.empty()) {
    Clear();
  }

  // Add states.
  states_.resize(n_states);
  for (int i = 0; i < n_states; ++i) {
    states_[i] = new State(i);
  }
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

void StateMachine::AddTransition(unsigned from_id, unsigned to_id,
                                 int event_id) {
  Transition* trans = new Transition(transitions_.size(),
                                     states_[from_id],
                                     states_[to_id],
                                     event_id);
  transitions_.push_back(trans);
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
                            const std::map<int, std::string>& events) const {
  const int n_states = states_.size();

  if (states_names.size() != n_states) {
    std::cout << "[StateMachine::WriteDot] Number of names must be same as "
                 "number of states. (" << states_names.size() << " vs. "
                 << n_states << ")." << std::endl;
    return;
  }

  std::ofstream file(file_path.c_str());
  file << "strict digraph state_machine {\n";

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

      const int n_trans = state->transitions.size();
      for (int j = 0; j < n_trans; ++j) {
        Transition* trans = state->transitions[j];
        const int state_to_id = trans->to->id;
        std::string state_to_name = states_names[state_to_id];
        std::string event_name = events.at(trans->event_id);

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

void StateMachine::WriteConfig(std::ofstream* s) const {
  *s << states_.size() << std::endl;

  const int n_trans = transitions_.size();
  *s << n_trans << std::endl;
  for (int i = 0; i < n_trans; ++i) {
    *s << transitions_[i]->from->id << ' '
       << transitions_[i]->to->id << ' '
       << transitions_[i]->event_id << std::endl;
  }
}
