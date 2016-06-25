// Copyright © 2016 Dmitry Kurtaev. All rights reserved.
// License: MIT License (see LICENSE)
// e-mail: dmitry.kurtaev@gmail.com

#ifndef INCLUDE_CODE_GENERATOR_H_
#define INCLUDE_CODE_GENERATOR_H_

#include <string>
#include <vector>

#include "include/state_machine.h"

class CodeGenerator {
 public:
  static int MinCodeLength(int max_elem_code_length, int n_elem_codes);

  static int MaxCodeLength(int max_elem_code_length, int n_elem_codes);

  static int MaxNumberElemCodes(int max_elem_code_length);

  static void GenCode(int code_length, int max_elem_code_length,
                      int n_elem_codes, std::vector<std::string>* code);

  static void GenStateMachine(int n_elem_codes, int n_states,
                              StateMachine* state_machine);

  static unsigned GetLNSetLimit(unsigned max_elem_code_length,
                                unsigned n_elem_codes);

  static std::string CodeFromBinary(unsigned data, unsigned length);

  static void GenPrefixCode(int max_elem_code_length, int n_elem_codes,
                            std::vector<std::string>* code);

 private:
  static void GenCodeLengths(int code_length, int max_elem_code_length,
                             int n_elem_codes, std::vector<int>* lengths);

  // Part of prefix code generation.
  static void GenLeafs(const std::string& node_code, int n_leafs,
                       int max_depth, std::vector<std::string>* codes);
};

#endif  // INCLUDE_CODE_GENERATOR_H_
