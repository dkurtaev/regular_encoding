#include "include/alphabetic_encoder.h"

#include <stdlib.h>
#include <stdio.h>

#include <queue>
#include <fstream>
#include <string>
#include <map>
#include <sstream>

const int AlphabeticEncoder::kStateMachineStartStateId = -1;
const int AlphabeticEncoder::kStateMachineEndStateId = -2;
const int AlphabeticEncoder::kEndCharacterId = -2;

AlphabeticEncoder::AlphabeticEncoder(const std::string& config_file) {
  // File format:
  // [int] alphabet size
  // [string] alphabet encoding
  // state machine description:
  // [number of states] - number of states, numerated from 0 to num - 1
  //                      (without start and finish states)
  // [number of transitions]
  // [int int int] - state id from,
  //                 state id to,
  //                 character id on transition (-2 for '.')
  // state id -1 is a start state
  // state id -2 is a finish state, for
  // reserved character '.' as end of word
  std::ifstream file(config_file.c_str());
  if (file.is_open()) {
    // Read alphabet encoding.
    int alphabet_size;
    file >> alphabet_size;
    elem_codes_.resize(alphabet_size);
    for (int i = 0; i < alphabet_size; ++i) {
      file >> elem_codes_[i];
    }

    // Read state machine.
    state_machine_.AddState(kStateMachineStartStateId);
    state_machine_.SetStartState(kStateMachineStartStateId);
    state_machine_.AddState(kStateMachineEndStateId);

    int number_states;
    file >> number_states;
    for (int i = 0; i < number_states; ++i) {
      state_machine_.AddState(i);
    }

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
}

bool AlphabeticEncoder::CheckBijective() {
  return bijective_checker.IsBijective(elem_codes_, state_machine_);
}

void AlphabeticEncoder::WriteCodeStateMachine(
  const std::string& file_path) const {
  // Set states names.
  std::map<int, std::string> states_names;
  states_names[kStateMachineStartStateId] = "start";
  states_names[kStateMachineEndStateId] = "end";
  int n_states = state_machine_.GetNumberStates();
  for (int i = 0; i < n_states - 2; ++i) {
    std::ostringstream ss;
    ss << i;
    states_names[i] = ss.str();
  }

  // Set transitions names.
  std::map<int, std::string> events_names;
  events_names[kEndCharacterId] = "";
  for (int i = 0; i < elem_codes_.size(); ++i) {
    events_names[i] = elem_codes_[i];
  }

  state_machine_.WriteDot(file_path, states_names, events_names);
}

void AlphabeticEncoder::WriteDeficitsStateMachine(
    const std::string& file_path) const {
  bijective_checker.WriteDeficitsStateMachine(file_path);
}
