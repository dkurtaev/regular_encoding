#include <vector>

#include <gtest/gtest.h>

#include "include/code_generator.h"

TEST(CodeGenerator, gen_code_lengths) {
  static const int kNumberGenerations = 10;

  std::vector<std::string> code;
  for (int M = 3; M <= 5; ++M) {
    int N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (int N = 1; N <= N_max; ++N) {
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
  static const int kNumberGenerations = 10;

  std::vector<std::string> code;
  for (int M = 3; M <= 5; ++M) {
    int N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (int N = 1; N <= N_max; ++N) {
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
