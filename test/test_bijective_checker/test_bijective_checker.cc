#include <math.h>

#include <vector>

#include <gtest/gtest.h>

#include "include/bijective_checker.h"
#include "include/code_generator.h"
#include "include/alphabetic_encoder.h"

void StateMachineOfAllWords(int n_words, StateMachine& state_machine) {
  state_machine.Clear();
  state_machine.AddState(AlphabeticEncoder::kStateMachineStartStateId);
  state_machine.SetStartState(AlphabeticEncoder::kStateMachineStartStateId);
  state_machine.AddState(AlphabeticEncoder::kStateMachineEndStateId);
  state_machine.AddState(0);
  for (int i = 0; i < n_words; ++i) {
    state_machine.AddTransition(AlphabeticEncoder::kStateMachineStartStateId,
                                0, i);
    state_machine.AddTransition(0, 0, i);
  }
  state_machine.AddTransition(0, AlphabeticEncoder::kStateMachineEndStateId,
                              AlphabeticEncoder::kEndCharacterId);
}

// This test for checking not bijective for codes of all words from LN set.
// (LN set - set of parameters L and N where code garanted does not satisfy
// McMillan's condition).
TEST(BijectiveChecker, all_words_codes_LN_set) {
  static const int kNumberGenerations = 10;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (int M = 3; M <= 5; ++M) {
    int N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (int N = 1; N <= N_max; ++N) {
      // Generate state machine for code of all words.
      StateMachineOfAllWords(N, state_machine);

      int L_min = CodeGenerator::MinCodeLength(M, N);
      int log2 = ceil(log(N) / log(2));
      int L_max = std::min(N * (log2 + 1) - (1 << log2) - 1,
                           CodeGenerator::MaxCodeLength(M, N));
      for (int L = L_min; L <= L_max; ++L) {
        for (int gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);

          ASSERT_FALSE(checker.IsBijective(code, state_machine));
        }
      }
    }
  }
}
