#ifndef INCLUDE_BIJECTIVE_CHECKER_H_
#define INCLUDE_BIJECTIVE_CHECKER_H_

#include <vector>
#include <queue>
#include <string>
#include <climits>

#include "include/state_machine.h"
#include "include/structures.h"
#include "include/code_tree.h"

enum Verdict { IS_BIJECTIVE, NOT_BIJECTIVE, WIDTH_OUT };

class BijectiveChecker {
 public:
  BijectiveChecker();

  ~BijectiveChecker();

  Verdict IsBijective(const std::vector<std::string>& code,
                      StateMachine& code_state_machine,
                      std::vector<int>* first_bad_word = 0,
                      std::vector<int>* second_bad_word = 0,
                      int loop_finder_width_limit = 100000);

  void WriteDeficitsStateMachine(const std::string& file_path);

 private:
  void BuildDeficitsStateMachine(const CodeTree& code_tree);

  void AddIsotropicDeficits(int deficit_id,
                            const CodeTree& code_tree,
                            std::queue<int>& deficits_up_to_build);

  void AddAntitropicDeficits(int deficit_id,
                             const CodeTree& code_tree,
                             std::queue<int>& deficits_up_to_build);

  struct SynonymyState {
    State* deficit;
    State* upper_state;
    State* lower_state;
    State* init_state;

    unsigned Hash(unsigned code_sm_n_states);

    static bool IsTrivial(const std::vector<int>& sequence, int next_char);
  };

  Verdict FindSynonymyLoop(const StateMachine& code_state_machine,
                           std::vector<int>* first_bad_word = 0,
                           std::vector<int>* second_bad_word = 0,
                           int loop_finder_width_limit = INT_MAX);

  void RemoveBottlenecks();

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
