#ifndef INCLUDE_ALPHABETIC_ENCODER_H_
#define INCLUDE_ALPHABETIC_ENCODER_H_

#include <vector>
#include <string>

#include "include/state_machine.h"

class AlphabeticEncoder {
 public:
  static const int kStateMachineStartStateId = -1;
  static const int kStateMachineEndStateId = -2;
  static const int kEndCharacterId = -2;

  explicit AlphabeticEncoder(const std::string& config_file);

  bool CheckBijective();

 private:
  StateMachine state_machine_;
  std::vector<std::string> elem_codes_;
};

#endif  // INCLUDE_ALPHABETIC_ENCODER_H_
