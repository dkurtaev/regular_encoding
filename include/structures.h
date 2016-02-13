#ifndef INCLUDE_STRUCTURES_H_
#define INCLUDE_STRUCTURES_H_

#include <stdlib.h>

#include <string>
#include <vector>

struct Suffix;
struct ElementaryCode {
  int id;
  std::string str;
  std::vector<Suffix*> suffixes;

  ElementaryCode(int id, const std::string& str);
};

struct Suffix {
  int id;
  int length;
  std::vector<ElementaryCode*> owners;

  Suffix(int id, int length,
         ElementaryCode* first_owner = 0);

  std::string str();
};

struct Transition;
struct State {
  unsigned id;
  std::vector<Transition*> transitions_to;  // Transitions to this state.
  std::vector<Transition*> transitions_from;  // Transitions from this state.

  explicit State(unsigned id) : id(id) {}

  // Returns state by event or 0 if transition not exists.
  State* DoTransition(int event_id);

  Transition* GetTransition(int event_id);

  bool DelTransitionFrom(int id);
  bool DelTransitionTo(int id);
};


struct Transition {
  unsigned id;
  State* from;
  State* to;
  int event_id;

  Transition(unsigned id, State* from, State* to, unsigned event_id);
};

inline int rand(int a, int b) {
  return rand() % (b - a + 1) + a;
}

#endif  // INCLUDE_STRUCTURES_H_
