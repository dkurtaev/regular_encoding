#include "include/alphabetic_encoder.h"

#include <stdlib.h>
#include <stdio.h>

#include <queue>
#include <fstream>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

AlphabeticEncoder::AlphabeticEncoder(const std::string& config_file) {
  // File format:
  // [int] alphabet size
  // [string] alphabet encoding
  // state machine description:
  // [number of states]
  // [number of transitions]
  // [int int int] - state id from,
  //                 state id to,
  //                 character id
  std::ifstream file(config_file.c_str());

  // Read alphabet encoding.
  int alphabet_size;
  file >> alphabet_size;
  elem_codes_.resize(alphabet_size);
  for (int i = 0; i < alphabet_size; ++i) {
    file >> elem_codes_[i];
  }

  int number_states;
  file >> number_states;
  state_machine_.Init(number_states);

  int number_transitions;
  file >> number_transitions;
  for (int i = 0; i < number_transitions; ++i) {
    int from_id;
    int to_id;
    int character_id;
    file >> from_id;
    file >> to_id;
    file >> character_id;
    state_machine_.AddTransition(from_id, to_id, character_id);
  }
}

bool AlphabeticEncoder::CheckBijective() {
  return bijective_checker.IsBijective(elem_codes_, state_machine_);
}

void AlphabeticEncoder::WriteCodeStateMachine(
    const std::string& file_path) const {
  WriteCodeStateMachine(file_path, elem_codes_, state_machine_);
}

void AlphabeticEncoder::WriteCodeStateMachine(
  const std::string& file_path,
  const std::vector<std::string>& code,
  const StateMachine& state_machine) {
  // Set states names.
  std::vector<std::string> states_names;
  states_names.push_back("start");
  int n_states = state_machine.GetNumberStates();
  for (int i = 2; i < n_states; ++i) {
    std::ostringstream ss;
    ss << i;
    states_names.push_back(ss.str());
  }
  states_names.push_back("end");
  state_machine.WriteDot(file_path, states_names, code);
}

void AlphabeticEncoder::WriteDeficitsStateMachine(
    const std::string& file_path) {
  bijective_checker.WriteDeficitsStateMachine(file_path);
}
