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

// This test for checking LN set limit correctness
// (outside it McMillan's equation is false).
TEST(BijectiveChecker, LN_set_limit) {
  static const unsigned kNumberGenerations = 100;

  std::vector<std::string> code;
  for (unsigned M = 3; M <= 6; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::GetLNSetLimit(M, N);
      for (unsigned L = L_min; L < L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);
          unsigned sum = 0;
          for (unsigned i = 0; i < N; ++i) {
            unsigned len = code[i].length();
            ASSERT_LE(len, M);
            sum += 1 << (M - len);
          }
          ASSERT_GT(sum, 1 << M);
        }
      }
    }
  }
}

// This test for checking not bijective for codes of all words from LN set.
// (LN set - set of parameters L and N where code garanted does not satisfy
// McMillan's condition).
TEST(BijectiveChecker, all_words_codes_outside_LN_set) {
  static const unsigned kNumberGenerations = 100;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 3; M <= 6; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      // Generate state machine for code of all words.
      StateMachineOfAllWords(N, state_machine);

      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::GetLNSetLimit(M, N);
      for (unsigned L = L_min; L <= L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);
          ASSERT_FALSE(checker.IsBijective(code, state_machine));
        }
      }
    }
  }
}

// TODO: Test for random generation and check bijective -> McMillan