#ifndef INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_
#define INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_

#include <string>
#include <vector>

#include "include/state_machine.h"

class UnbijectiveCodeGenerator {
 public:
  static void Generate(std::vector<std::string>& code,
                       StateMachine& state_machine);

 private:
  static void GenDelimeters(int seed_length,
                            std::vector<std::vector<int> >& delimeters);

  static void ExtractElemCodes(const std::string& seed,
                               std::vector<std::vector<int> >& delimeters,
                               std::vector<std::vector<int> >& codes,
                               std::vector<std::string>& elem_codes);

  // Generate state machine recognizes generated decompositions.
  static void GenStateMachine(const int& n_elem_codes,
                              std::vector<std::vector<int> >& codes,
                              StateMachine& state_machine);
};

#endif  // INCLUDE_UNBIJECTIVE_CODE_GENERATOR_H_