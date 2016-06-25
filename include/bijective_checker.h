// Copyright © 2016 Dmitry Kurtaev. All rights reserved.
// License: MIT License (see LICENSE)
// e-mail: dmitry.kurtaev@gmail.com

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

  ~BijectiveChecker();

  bool IsBijective(const std::vector<std::string>& code,
                   const StateMachine& code_state_machine,
                   std::vector<int>* first_bad_word = 0,
                   std::vector<int>* second_bad_word = 0);

  void WriteDeficitsStateMachine(const std::string& file_path);

  void WriteSynonymyStateMachine(const std::string& file_path);

 private:
  void BuildDeficitsStateMachine(const CodeTree& code_tree);

  void AddIsotropicDeficits(int deficit_id,
                            const CodeTree& code_tree,
                            std::queue<int>* deficits_up_to_build);

  void AddAntitropicDeficits(int deficit_id,
                             const CodeTree& code_tree,
                             std::queue<int>* deficits_up_to_build);

  void BuildSynonymyStateMachine();

  struct SynonymyState {
    State* deficit;
    State* upper_state;
    State* lower_state;
    int* sequence;
    bool is_tivial;

    unsigned Hash(unsigned n_code_sm_states);

    static unsigned Hash(unsigned deficit_id, unsigned upper_state_id,
                         unsigned lower_state_id, unsigned n_code_sm_states);
  };

  bool FindSynonymyLoop(std::vector<int>* first_bad_word = 0,
                        std::vector<int>* second_bad_word = 0);

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
  StateMachine* synonymy_state_machine_;
  // Just reference for private methods.
  const StateMachine* code_state_machine_;
};

#endif  // INCLUDE_BIJECTIVE_CHECKER_H_
