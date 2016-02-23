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
                   const StateMachine& code_state_machine,
                   std::vector<int>* first_bad_word = 0,
                   std::vector<int>* second_bad_word = 0);

  void WriteDeficitsStateMachine(const std::string& file_path);

 private:
  // enum WordLocation { UPPER, LOWER };
  struct LoopState {
    std::vector<bool> deficits_transitions_trace;
    // std::vector<bool> words_trace[2];
    std::vector<int> words[2];
    // State* words_states[2];
    State* deficit_state;
  };

  void BuildDeficitsStateMachine(const CodeTree& code_tree);

  void AddIsotropicDeficits(int deficit_id,
                            const CodeTree& code_tree,
                            std::queue<int>& deficits_up_to_build);

  void AddAntitropicDeficits(int deficit_id,
                             const CodeTree& code_tree,
                             std::queue<int>& deficits_up_to_build);

  void LogDeficitsBuilding(int state_id_from,
                           int state_id_to,
                           ElementaryCode* elem_code);

  // bool FindTargetLoop(const StateMachine& code_state_machine,
  //                     std::vector<int>* first_bad_word,
  //                     std::vector<int>* second_bad_word);

  bool AlternativeFindTargetLoop(const StateMachine& code_state_machine,
                                 std::vector<int>* first_bad_word,
                                 std::vector<int>* second_bad_word);

  bool ProcessLoopState(const StateMachine& code_state_machine,
                        LoopState* loop_state,
                        std::queue<LoopState*>& loop_states);

  // Returns true if target loop founded.
  // bool ProcessLoopTransition(LoopState* state,
  //                            Transition* def_transition,
  //                            std::queue<LoopState*>& states,
  //                            unsigned end_state_id);

  // void RemoveDeadTransitions(const StateMachine& code_state_machine);

  void RemoveBottlenecks();

  // bool DeficitsMachineIsTrivial();

  void Reset();

  // From (-3 -2 -1 0 1 2 3)
  // To (0 1 2 3 4 5 6 7)
  inline unsigned UnsignedDeficitId(int id);

  // From (0 1 2 3 4 5 6 7)
  // To (-3 -2 -1 0 1 2 3)
  inline int SignedDeficitId(unsigned id);

  std::vector<ElementaryCode*> code_;
  std::vector<Suffix*> code_suffixes_;
  StateMachine* deficits_state_machine_;
};

#endif  // INCLUDE_BIJECTIVE_CHECKER_H_
