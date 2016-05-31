#include <vector>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "include/bijective_checker.h"
#include "include/code_generator.h"
#include "include/structures.h"
#include "include/unbijective_code_generator.h"
#include "test/macros.h"

static const unsigned kNumberGenerations = 25;

void StateMachineOfAllWords(int n_words, StateMachine& state_machine) {
  state_machine.Init(1);
  for (int i = 0; i < n_words; ++i) {
    state_machine.AddTransition(0, 0, i);
  }
}

// This test for checking not bijective for codes of all words from LN set.
// (LN set - set of parameters L and N where code garanted does not satisfy
// McMillan's condition).
TEST(BijectiveChecker, all_words_codes_outside_LN_set) {
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
          CodeGenerator::GenCode(L, M, N, &code);
          ASSERT_FALSE(checker.IsBijective(code, state_machine));

          std::ostringstream ss;
          ss << "BijectiveChecker.all_words_codes_outside_LN_set: Processed M="
             << M << ", N=" << N << ", L=" << L;
          Log(ss.str(), 300);
        }
      }
    }
  }
}

// This test for generating all words code, check bijectivity and if
// it is bijective, check McMillan's condition.
TEST(BijectiveChecker, all_words_codes_mcmillan) {
  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 2; M <= 5; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      StateMachineOfAllWords(N, state_machine);

      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::MaxCodeLength(M, N);
      for (unsigned L = L_min; L <= L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, &code);

          if (checker.IsBijective(code, state_machine)) {
            unsigned sum = 0;
            for (unsigned i = 0; i < N; ++i) {
              unsigned len = code[i].length();
              ASSERT_LE(len, M);
              sum += 1 << (M - len);
            }
            ASSERT_LE(sum, 1 << M);
          }

          std::ostringstream ss;
          ss << "BijectiveChecker.all_words_codes_mcmillan: Processed M="
             << M << ", N=" << N << ", L=" << L;
          Log(ss.str(), 300);
        }
      }
    }
  }
}

// Generating prefix codes and random state machine. Check that are bijective.
TEST(BijectiveChecker, prefix_codes) {
  static const unsigned kNumberCodeGens = 3;
  static const unsigned kMaxNumberStates = 5;
  static const unsigned kNumberMachineGens = 3;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 2; M <= 4; ++M) {
    unsigned N_max = 1 << M;
    for (unsigned N = 2; N <= N_max; ++N) {
      for (unsigned i = 0; i < kNumberCodeGens; ++i) {
        CodeGenerator::GenPrefixCode(M, N, &code);
        for (unsigned n_states = 1; n_states <= kMaxNumberStates; ++n_states) {
          for (unsigned j = 0; j < kNumberMachineGens; ++j) {
            CodeGenerator::GenStateMachine(N, n_states, &state_machine);
            ASSERT_TRUE(checker.IsBijective(code, state_machine));
            
            std::ostringstream ss;
            ss << "BijectiveChecker.prefix_codes: Processed M="
               << M << ", N=" << N;
            Log(ss.str(), 300);
          }
        }
      }
    }
  }
}

TEST(BijectiveChecker, checker_output) {
  static const unsigned kNumberCodeGens = 3;
  static const unsigned kMaxNumberStates = 5;
  static const unsigned kNumberMachineGens = 3;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  std::vector<int> first_bad_word;
  std::vector<int> second_bad_word;
  for (unsigned M = 2; M <= 5; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::MaxCodeLength(M, N);
      for (unsigned L = L_min; L <= L_max; ++L) {
        for (unsigned i = 0; i < kNumberCodeGens; ++i) {
          CodeGenerator::GenCode(L, M, N, &code);
          for (int n_states = 1; n_states <= kMaxNumberStates; ++n_states) {
            for (unsigned j = 0; j < kNumberMachineGens; ++j) {
              CodeGenerator::GenStateMachine(N, n_states, &state_machine);
              bool is_bijective = checker.IsBijective(code, state_machine,
                                                      &first_bad_word,
                                                      &second_bad_word);
              if (!is_bijective) {
                ASSERT_NE(first_bad_word.size(), 0);
                ASSERT_NE(second_bad_word.size(), 0);

                if (first_bad_word.size() == second_bad_word.size()) {
                  int length = first_bad_word.size();
                  bool words_are_different = false;
                  for (int i = 0; i < length; ++i) {
                    if (first_bad_word[i] != second_bad_word[i]) {
                      words_are_different = true;
                      break;
                    }
                  }
                  ASSERT_TRUE(words_are_different);
                }
                ASSERT_TRUE(state_machine.IsRecognized(first_bad_word));
                ASSERT_TRUE(state_machine.IsRecognized(second_bad_word));

                std::string first_word = "";
                for (int i = 0; i < first_bad_word.size(); ++i) {
                  first_word += code[first_bad_word[i]];
                }
                std::string second_word = "";
                for (int i = 0; i < second_bad_word.size(); ++i) {
                  second_word += code[second_bad_word[i]];
                }
                ASSERT_EQ(first_word, second_word);
              } else {
                ASSERT_EQ(first_bad_word.size(), 0);
                ASSERT_EQ(second_bad_word.size(), 0);
              }

              std::ostringstream ss;
              ss << "BijectiveChecker.checker_output: Processed M="
                 << M << ", N=" << N << ", L=" << L;
              Log(ss.str(), 300);
            }
          }
        }
      }
    }
  }
}

// Testing that method can find not bijective codes.
TEST(BijectiveChecker, not_bijective_codes) {
  static const int kNumberGenerations = 10000;

  std::vector<std::string> code;
  StateMachine state_machine;
  BijectiveChecker checker;
  for (int i = 0; i < kNumberGenerations; ++i) {
    UnbijectiveCodeGenerator::Generate(&code, &state_machine);
    ASSERT_FALSE(checker.IsBijective(code, state_machine));
  }
}
