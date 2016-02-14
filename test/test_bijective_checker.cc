#include <vector>
#include <sys/time.h>

#include <gtest/gtest.h>

#include "include/bijective_checker.h"
#include "include/code_generator.h"

void StateMachineOfAllWords(int n_words, StateMachine& state_machine) {
  state_machine.Clear();
  state_machine.AddStates(1);
  for (int i = 0; i < n_words; ++i) {
    state_machine.AddTransition(0, 0, i);
  }
}

// This test for checking not bijective for codes of all words from LN set.
// (LN set - set of parameters L and N where code garanted does not satisfy
// McMillan's condition).
TEST(BijectiveChecker, all_words_codes_outside_LN_set) {
  static const unsigned kNumberGenerations = 25;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 2; M <= 5; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      // Generate state machine for code of all words.
      StateMachineOfAllWords(N, state_machine);

      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::GetLNSetLimit(M, N);
      for (unsigned L = L_min; L < L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);
          ASSERT_FALSE(checker.IsBijective(code, state_machine));
        }
      }
    }
  }
}

// This test for generating all words code, check bijectivity and if
// it is bijective, check McMillan's condition.
TEST(BijectiveChecker, all_words_codes_mcmillan) {
  static const unsigned kNumberGenerations = 25;

  timeval last_log_time;
  gettimeofday(&last_log_time, 0);

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 6; M <= 6; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      StateMachineOfAllWords(N, state_machine);

      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::MaxCodeLength(M, N);
      for (unsigned L = L_min; L <= L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);
          if (checker.IsBijective(code, state_machine)) {
            unsigned sum = 0;
            for (unsigned i = 0; i < N; ++i) {
              unsigned len = code[i].length();
              ASSERT_LE(len, M);
              sum += 1 << (M - len);
            }
            ASSERT_LE(sum, 1 << M);
          }
        }
        timeval current_time;
        gettimeofday(&current_time, 0);
        if (current_time.tv_sec - last_log_time.tv_sec >= 60) {
          std::cout << "Processed M = " << M << " N = " << N << " L = " << L
                    << std::endl;
          last_log_time = current_time;
        }
      }
    }
  }
}
