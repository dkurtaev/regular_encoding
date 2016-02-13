#ifndef INCLUDE_ALPHABETIC_ENCODER_H_
#define INCLUDE_ALPHABETIC_ENCODER_H_

#include <vector>
#include <string>

#include "include/state_machine.h"
#include "include/bijective_checker.h"

class AlphabeticEncoder {
 public:
//  static const int kEndCharacterId;

  explicit AlphabeticEncoder(const std::string& config_file);

  bool CheckBijective();

  void WriteCodeStateMachine(const std::string& file_path) const;

  void WriteDeficitsStateMachine(const std::string& file_path);

 private:
  BijectiveChecker bijective_checker;

  // First state of state machine - start state.
  // Second state of state machine - end state.
  // Other states starts from third state.
  //
  // Events id - elementary code id.
  // Event for end character - next available after elementary codes.
  StateMachine state_machine_;
  std::vector<std::string> elem_codes_;
};

#endif  // INCLUDE_ALPHABETIC_ENCODER_H_
