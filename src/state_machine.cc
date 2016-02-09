#include "include/state_machine.h"

#include <iostream>

StateMachine::StateMachine()
  : start_state_(0),
    n_states_(0),
    n_transitions_(0) {
}

bool StateMachine::AddState(int id) {
  if (states_.find(id) == states_.end()) {
    ++n_states_;
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
  n_states_ = 0;
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
  return n_states_;
}

int StateMachine::GetNumberTransitions() const {
  return n_transitions_;
}

void StateMachine::Log(std::ostream* s) const {
  *s << ' ' << n_states_ - 2 << ' ' << n_transitions_;
  std::map<int, State*>::const_iterator it;
  for (it = states_.begin(); it != states_.end(); ++it) {
    for (int i = 0; i < it->second->transitions.size(); ++i) {
      *s << ' ' << it->second->id
         << ' ' << it->second->transitions[i]->to->id
         << ' ' << it->second->transitions[i]->event_id;
    }
  }
}
