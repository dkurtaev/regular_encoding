#ifndef INCLUDE_STATE_MACHINE_H_
#define INCLUDE_STATE_MACHINE_H_

#include <vector>
#include <map>
#include <string>

#include "include/structures.h"

class StateMachine {
 public:
  StateMachine(int n_states = 0);

  ~StateMachine();

  void Clear();

  void AddStates(int n_states);

  void AddTransition(unsigned from_id, unsigned to_id, unsigned event_id);

  void DelTransition(unsigned id);

  void DelState(unsigned id);

  // First added state is start state.
  State* GetStartState() const;

  // Last added state.
  State* GetEndState() const;

  State* GetState(int id) const;

  int GetNumberStates() const;

  int GetNumberTransitions() const;

  void WriteDot(const std::string& file_path,
                const std::vector<std::string>& states_names,
                const std::vector<std::string>& events_names) const;

 private:
  std::vector<State*> states_;
  std::vector<Transition*> transitions_;

  // Used for tracking deleted states and transitions. If we delete state, we
  // not erase it from vector.
  int n_states_;
  int n_transitions_;
};

#endif  // INCLUDE_STATE_MACHINE_H_
