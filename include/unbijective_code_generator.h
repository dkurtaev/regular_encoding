#ifndef INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_
#define INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_

#include <string>
#include <vector>

#include "include/state_machine.h"

class UnbijectiveCodeGenerator {
 public:
  static void Generate(std::vector<std::string>* code,
                       StateMachine* state_machine);

 private:
  static void GenDelimeters(int seed_length, std::vector<int>* delimeters);

  static void ExtractElemCodes(const std::string& seed,
                               std::vector<int>* delimeters,
                               std::vector<int>* codes,
                               std::vector<std::string>* elem_codes);

  // Generate state machine recognizes generated decompositions.
  static void GenStateMachine(int n_elem_codes,
                              const std::vector<int>* codes,
                              StateMachine* state_machine);
};

#endif  // INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_
