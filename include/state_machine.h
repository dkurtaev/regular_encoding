#ifndef INCLUDE_STATE_MACHINE_H_
#define INCLUDE_STATE_MACHINE_H_

#include <vector>
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

  Transition* GetTransition(int id) const;

  int GetNumberStates() const;

  int GetNumberTransitions() const;

  bool IsRecognized(const std::vector<int>& word) const;

  bool FindContext(std::vector<int>& first_substr,
                   std::vector<int>& second_substr) const;

  void WriteDot(const std::string& file_path,
                const std::vector<std::string>& states_names,
                const std::vector<std::string>& events_names) const;

 private:
  typedef std::pair<State*, State*> StatesPair;

  bool FindSubwords(const std::vector<int>& word,
                    std::vector<StatesPair>& pairs) const;

  bool FindAnyPath(int from, int to, std::vector<int>& word) const;

  // Find set of characters which achieves end state for both input states.
  bool FindEndOfContext(State* first_state, State* second_state,
                        std::vector<int>& end_context) const;

  inline void InsertBack(std::vector<int>& dst,
                         const std::vector<int>& src) const;

  inline void InsertFront(const std::vector<int>& src,
                          std::vector<int>& dst) const;

  std::vector<State*> states_;
  std::vector<Transition*> transitions_;
};

#endif  // INCLUDE_STATE_MACHINE_H_
