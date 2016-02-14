#include "include/structures.h"

#include <iostream>

ElementaryCode::ElementaryCode(int id, const std::string& str)
  : id(id),
    str(str) {
}

Suffix::Suffix(int id, int length,
               ElementaryCode* first_owner)
  : id(id),
    length(length) {
  if (first_owner) {
    owners.push_back(first_owner);
  }
}

std::string Suffix::str() {
  return owners[0]->str.substr(owners[0]->str.length() - length);
}

Transition::Transition(unsigned id, State* from, State *to, unsigned event_id)
  : from(from),
    to(to),
    event_id(event_id),
    id(id) {
  from->transitions_from.push_back(this);
  to->transitions_to.push_back(this);
}

State* State::DoTransition(int event_id) {
  for (int i = 0; i < transitions_from.size(); ++i) {
    if (transitions_from[i]->event_id == event_id) {
      return transitions_from[i]->to;
    }
  }
  return 0;
}

Transition* State::GetTransition(int event_id) {
  for (int i = 0; i < transitions_from.size(); ++i) {
    if (transitions_from[i]->event_id == event_id) {
      return transitions_from[i];
    }
  }
  return 0;
}

bool State::DelTransitionFrom(int id) {
  std::vector<Transition*>::iterator it;
  for (it = transitions_from.begin(); it != transitions_from.end(); ++it) {
    Transition* transition = *it;
    if (transition->id == id) {
      transitions_from.erase(it);
      return true;
    }
  }
  return false;
}

bool State::DelTransitionTo(int id) {
  std::vector<Transition*>::iterator it;
  for (it = transitions_to.begin(); it != transitions_to.end(); ++it) {
    Transition* transition = *it;
    if (transition->id == id) {
      transitions_to.erase(it);
      return true;
    }
  }
  return false;
}
