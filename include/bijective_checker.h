#ifndef INCLUDE_BIJECTIVE_CHECKER_H_
#define INCLUDE_BIJECTIVE_CHECKER_H_

#include <vector>
#include <queue>
#include <string>

#include "include/state_machine.h"
#include "include/structures.h"
#include "include/code_tree.h"

class BijectiveChecker {
 public:
  BijectiveChecker();

  bool IsBijective(const std::vector<std::string>& code,
                   const StateMachine& code_state_machine);

 private:
  struct LoopState {
    // Visited transitions in deficits state machine.
    std::vector<bool> deficits_trace;
    // Visited transitions in code state machine for upper word.
    std::vector<bool> upper_word_trace;
    // Visited transitions in code state machine for upper word.
    std::vector<bool> lower_word_trace;
    std::vector<int> upper_word;
    std::vector<int> lower_word;
    State* upper_word_state;
    State* lower_word_state;
    State* deficit_state;
  };

  void BuildDeficitsStateMachine();

  void AddIsotropicDeficits(int deficit_id,
                            std::queue<int>& deficits_up_to_build);

  void AddAntitropicDeficits(int deficit_id,
                             std::queue<int>& deficits_up_to_build);

  void LogDeficitsBuilding(int state_id_from,
                           int state_id_to,
                           ElementaryCode* elem_code);

  bool FindTargetLoop(const StateMachine& code_state_machine);

  // Returns true if target loop founded.
  bool ProcessLoopTransition(LoopState* state,
                             Transition* def_transition,
                             std::queue<LoopState*>* states);

  void Reset();

  std::vector<ElementaryCode*> code_;
  std::vector<Suffix*> code_suffixes_;
  CodeTree* code_tree_;
  StateMachine* deficits_state_machine_;
};

#endif  // INCLUDE_BIJECTIVE_CHECKER_H_
