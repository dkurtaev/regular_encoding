#include "include/structures.h"

#include <iostream>
#include <algorithm>

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

Transition::Transition(unsigned id, State* from, State *to, int event_id)
  : from(from),
    to(to),
    event_id(event_id),
    id(id) {
  from->transitions_from.push_back(this);
  to->transitions_to.push_back(this);
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

void GenUniqueUnnegatives(int upper_value, int number,
                          std::vector<int>& values) {
  if (number < upper_value / 2) {
    std::vector<int> available_values(upper_value + 1);
    for (int i = 0; i <= upper_value; ++i) {
      available_values[i] = i;
    }
    values.resize(number);
    int idx;
    for (int i = 0; i < number; ++i) {
      idx = rand() % (upper_value + 1 - i);
      values[i] = available_values[idx];
      available_values.erase(available_values.begin() + idx);
    }
  } else {
    values.resize(upper_value + 1);
    for (int i = 0; i <= upper_value; ++i) {
      values[i] = i;
    }
    int idx;
    for (int i = 0; i < upper_value + 1 - number; ++i) {
      idx = rand() % (upper_value + 1 - i);
      values.erase(values.begin() + idx);
    }
  }
  std::sort(values.begin(), values.end());
}

void InsertBack(std::vector<int>& dst, const std::vector<int>& src) {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.end(), src.begin(), src.end());
  }
}

void InsertFront(const std::vector<int>& src, std::vector<int>& dst) {
  if (src.size() != 0) {
    dst.reserve(dst.size() + src.size());
    dst.insert(dst.begin(), src.begin(), src.end());
  }
}
