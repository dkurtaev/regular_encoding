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

  Suffix(int id, int length, ElementaryCode* first_owner = 0);

  std::string str();
};

struct Transition;
struct State {
  unsigned id;
  std::vector<Transition*> transitions;  // Transitions from this state.

  explicit State(unsigned id) : id(id) {}

  Transition* GetTransition(int event_id);
};


struct Transition {
  unsigned id;
  State* from;
  State* to;
  int event_id;

  Transition(unsigned id, State* from, State* to, int event_id);
};

inline int rand(int a, int b) {
  return rand() % (b - a + 1) + a;
}

void GenUniqueUnnegatives(int upper_value, int number,
                          std::vector<int>* values);

template<typename T>
void InsertBack(std::vector<T>* dst, const std::vector<T>& src) {
  if (src.size() != 0) {
    dst->reserve(dst->size() + src.size());
    dst->insert(dst->end(), src.begin(), src.end());
  }
}

template<typename T>
void InsertFront(const std::vector<T>& src, std::vector<T>* dst) {
  if (src.size() != 0) {
    dst->reserve(dst->size() + src.size());
    dst->insert(dst->begin(), src.begin(), src.end());
  }
}

#endif  // INCLUDE_STRUCTURES_H_
