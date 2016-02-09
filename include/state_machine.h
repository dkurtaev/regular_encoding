#ifndef INCLUDE_STATE_MACHINE_H_
#define INCLUDE_STATE_MACHINE_H_

#include <vector>
#include <map>

#include "include/structures.h"

class StateMachine {
 public:
  StateMachine();

  ~StateMachine();

  bool AddState(int state_id);

  bool AddTransition(int from_id, int to_id, int event_id);

  bool SetStartState(int state_id);

  State* GetStartState() const;

  int GetNumberStates() const;

  int GetNumberTransitions() const;

  void Clear();

  void Log(std::ostream* s) const;

 private:
  State* start_state_;
  std::map<int, State*> states_;
  int n_states_;
  int n_transitions_;
};

#endif  // INCLUDE_STATE_MACHINE_H_
