#include "include/structures.h"

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

Transition::Transition(int id, State* from, State *to, int event_id)
  : from(from),
    to(to),
    event_id(event_id),
    id(id) {
}

State::State(int id)
  : id(id) {
}

State* State::DoTransition(int event_id) {
  for (int i = 0; i < transitions.size(); ++i) {
    if (transitions[i]->event_id == event_id) {
      return transitions[i]->to;
    }
  }
  return 0;
}

Transition* State::GetTransition(int event_id) {
  for (int i = 0; i < transitions.size(); ++i) {
    if (transitions[i]->event_id == event_id) {
      return transitions[i];
    }
  }
  return 0;
}
