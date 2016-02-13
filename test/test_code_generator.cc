#include <vector>

#include <gtest/gtest.h>

#include "include/code_generator.h"

TEST(CodeGenerator, gen_code_lengths) {
  static const int kNumberGenerations = 25;

  std::vector<std::string> code;
  for (int M = 3; M <= 6; ++M) {
    int N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (int N = 2; N <= N_max; ++N) {
      int L_min = CodeGenerator::MinCodeLength(M, N);
      int L_max = CodeGenerator::MaxCodeLength(M, N);
      for (int L = L_min; L <= L_max; ++L) {
        for (int gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);

          ASSERT_EQ(code.size(), N);
          int max_length = 0;
          int code_length = 0;
          for (int i = 0; i < code.size(); ++i) {
            int length = code[i].length();
            code_length += length;
            if (length > max_length) {
              max_length = code[i].length();
            }
          }
          ASSERT_EQ(code_length, L);
          ASSERT_EQ(max_length, M);
        }
      }
    }
  }
}

TEST(CodeGenerator, elem_codes_uniqueness) {
  static const int kNumberGenerations = 25;

  std::vector<std::string> code;
  for (int M = 3; M <= 6; ++M) {
    int N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (int N = 2; N <= N_max; ++N) {
      int L_min = CodeGenerator::MinCodeLength(M, N);
      int L_max = CodeGenerator::MaxCodeLength(M, N);
      for (int L = L_min; L <= L_max; ++L) {
        for (int gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);

          for (int i = 0; i < code.size(); ++i) {
            for (int j = 0; j < i; ++j) {
              ASSERT_NE(code[i], code[j]);
            }
            for (int j = i + 1; j < code.size(); ++j) {
              ASSERT_NE(code[i], code[j]);
            }
          }
        }
      }
    }
  }
}

// This test for checking LN set limit correctness
// (outside it McMillan's equation is false).
TEST(CodeGenerator, LN_set_limit) {
  static const unsigned kNumberGenerations = 25;

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
