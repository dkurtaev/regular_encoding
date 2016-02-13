#include "include/state_machine.h"

#include <fstream>

StateMachine::StateMachine()
  : start_state_(0),
    n_transitions_(0) {
}

bool StateMachine::AddState(int id) {
  if (states_.find(id) == states_.end()) {
    states_[id] = new State(id);
    return true;
  } else {
    return false;
  }
}

bool StateMachine::AddTransition(int from_id, int to_id, int event_id) {
  State* from = states_[from_id];
  State* to = states_[to_id];
  if (from != 0 & to != 0) {
    from->transitions.push_back(new Transition(n_transitions_, to, event_id));
    ++n_transitions_;
    return true;
  } else {
    return false;
  }
}

bool StateMachine::SetStartState(int id) {
  std::map<int, State*>::iterator it = states_.find(id);
  if (it != states_.end()) {
    start_state_ = (*it).second;
    return true;
  } else {
    return false;
  }
}

void StateMachine::Clear() {
  std::map<int, State*>::iterator it;
  for (it = states_.begin(); it != states_.end(); ++it) {
    delete it->second;
  }
  states_.clear();
  n_transitions_ = 0;
  start_state_ = 0;
}

StateMachine::~StateMachine() {
  Clear();
}

State* StateMachine::GetStartState() const {
  return start_state_;
}

int StateMachine::GetNumberStates() const {
  return states_.size();
}

int StateMachine::GetNumberTransitions() const {
  return n_transitions_;
}

void StateMachine::WriteDot(
        const std::string& file_path,
        const std::map<int, std::string>& states_names,
        const std::map<int, std::string>& events_names) const {
  std::ofstream file(file_path.c_str());
  file << "strict digraph state_machine {\n";

  std::map<std::pair<int, int>, std::string> edges;
  std::map<int, State*>::const_iterator it;

  for (it = states_.begin(); it != states_.end(); ++it) {
    int state_from_id = it->second->id;
    std::string state_from = states_names.find(state_from_id)->second;
    for (int i = 0; i < it->second->transitions.size(); ++i) {
      int state_to_id = it->second->transitions[i]->to->id;
      int event_id = it->second->transitions[i]->event_id;
      std::string state_to = states_names.find(state_to_id)->second;
      std::string event = events_names.find(event_id)->second;

      std::pair<int, int> edge_id(state_from_id, state_to_id);
      if (edges.find(edge_id) == edges.end()) {
        edges[edge_id] = event;
      } else {
        edges[edge_id] += ", " + event;
      }

      file << state_from << "->" << state_to
           << "[label=\"" << edges[edge_id] << "\"];\n";
    }
  }

  file << "}";
  file.close();
}
