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
                   StateMachine& code_state_machine,
                   std::vector<int>* first_bad_word = 0,
                   std::vector<int>* second_bad_word = 0);

  void WriteDeficitsStateMachine(const std::string& file_path);

 private:
  void BuildDeficitsStateMachine(const CodeTree& code_tree);

  void AddIsotropicDeficits(int deficit_id,
                            const CodeTree& code_tree,
                            std::queue<int>& deficits_up_to_build);

  void AddAntitropicDeficits(int deficit_id,
                             const CodeTree& code_tree,
                             std::queue<int>& deficits_up_to_build);

  bool FindTargetLoop(const StateMachine& code_state_machine,
                      std::vector<int>* first_bad_word,
                      std::vector<int>* second_bad_word);

  bool ProcessNextPath(const StateMachine& code_state_machine,
                       std::queue<std::vector<Transition*>* >& paths,
                       std::queue<bool*>& visited_states,
                       std::vector<int>* first_bad_word,
                       std::vector<int>* second_bad_word);

  struct SynonymyState {
    State* deficit;
    State* upper_state;
    State* lower_state;
    std::vector<int> words_pair;
  };

  void BuildSynonymyStateMachine(StateMachine& code_state_machine);

  void RemoveBottlenecks();

  void Reset();

  void CollectWords(const std::vector<Transition*>& path,
                    std::vector<int>& first_word,
                    std::vector<int>& second_word);

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
