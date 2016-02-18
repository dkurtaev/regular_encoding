#include <vector>

#include <gtest/gtest.h>

#include "include/code_generator.h"

static const int kNumberGenerations = 25;

#define GEN_LOOP(M, N, L) \
for (int M = 2; M <= 6; ++M) { \
  int N_max = CodeGenerator::MaxNumberElemCodes(M); \
  for (int N = 2; N <= N_max; ++N) { \
    int L_min = CodeGenerator::MinCodeLength(M, N); \
    int L_max = CodeGenerator::MaxCodeLength(M, N); \
    for (int L = L_min; L <= L_max; ++L) { \
      for (int gen = 0; gen < kNumberGenerations; ++gen)

#define END_GEN_LOOP }}}

TEST(CodeGenerator, gen_code_lengths) {
  std::vector<std::string> code;
  GEN_LOOP(M, N, L) {
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
  END_GEN_LOOP
}

TEST(CodeGenerator, elem_codes_uniqueness) {
  std::vector<std::string> code;
  GEN_LOOP(M, N, L) {
    CodeGenerator::GenCode(L, M, N, code);
    for (int i = 0; i < code.size(); ++i) {
      for (int j = i + 1; j < code.size(); ++j) {
        ASSERT_NE(code[i], code[j]);
      }
    }
  }
  END_GEN_LOOP
}

// This test for checking LN set limit correctness
// (outside it McMillan's equation is false).
TEST(CodeGenerator, LN_set_limit) {
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

TEST(CodeGenerator, prefix_codes_lengths) {
  std::vector<std::string> code;
  for (unsigned M = 2; M <= 7; ++M) {
    unsigned N_max = 1 << M;
    for (unsigned N = 2; N <= N_max; ++N) {
      for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
        CodeGenerator::GenPrefixCode(M, N, code);
        ASSERT_EQ(code.size(), N);
        int max_length = 0;
        for (int i = 0; i < code.size(); ++i) {
          max_length = std::max(length, code[i].length());
        }
        ASSERT_LE(max_length, M);
      }
    }
  }
}

// This test for checking that prefix codes are really prefix.
TEST(CodeGenerator, prefix_codes_are_prefix) {
  std::vector<std::string> code;
  for (unsigned M = 2; M <= 7; ++M) {
    unsigned N_max = 1 << M;
    for (unsigned N = 2; N <= N_max; ++N) {
      for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
        CodeGenerator::GenPrefixCode(M, N, code);
        for (int i = 0; i < N; ++i) {
          for (int j = i + 1; j < N; ++j) {
            ASSERT_NE(code[i].find(code[j]), 0);
            ASSERT_NE(code[j].find(code[i]), 0);
          }
        }
      }
    }
  }
}
