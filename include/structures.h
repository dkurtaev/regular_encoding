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
  int id;
  std::vector<Transition*> transitions;

  explicit State(int id);

  // Returns state by event or 0 if transition not exists.
  State* DoTransition(int event_id);

  Transition* GetTransition(int event_id);
};

struct Transition {
  int id;
  State* from;
  State* to;
  int event_id;

  Transition(int id, State* from, State* to, int event_id);
};

inline int rand(int a, int b) {
  return rand() % (b - a + 1) + a;
}

#endif  // INCLUDE_STRUCTURES_H_
