#include "include/state_machine.h"

#include <fstream>

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

  int n_trans = transitions_.size();
  for (int i = 0; i < n_trans; ++i) {
    delete transitions_[i];
  }
  transitions_.clear();
}

void StateMachine::AddStates(int n_states) {
  for (int i = 0; i < n_states; ++i) {
    states_.push_back(new State(i));
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
  transitions_[id]->from->DelTransitionFrom(id);
  transitions_[id]->to->DelTransitionTo(id);
}

void StateMachine::DelState(unsigned id) {
  State* state = states_[id];
  for (int i = 0; i < state->transitions_from.size(); ++i) {
    DelTransition(state->transitions_from[i]->id);
  }
  for (int i = 0; i < state->transitions_to.size(); ++i) {
    DelTransition(state->transitions_to[i]->id);
  }
}

State* StateMachine::GetStartState() const {
  return states_[0];
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
    const int n_trans = state->transitions_from.size();
    std::string state_from_name = states_names[i];
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

  file << "}";
  file.close();
}
