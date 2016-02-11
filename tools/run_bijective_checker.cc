#include <iostream>

#include "include/code_generator.h"
#include "include/bijective_checker.h"
#include "include/state_machine.h"
#include "include/alphabetic_encoder.h"

void StateMachineOfAllWords(int n_words, StateMachine& state_machine) {
  state_machine.Clear();
  state_machine.AddState(AlphabeticEncoder::kStateMachineStartStateId);
  state_machine.SetStartState(AlphabeticEncoder::kStateMachineStartStateId);
  state_machine.AddState(AlphabeticEncoder::kStateMachineEndStateId);
  state_machine.AddState(0);
  for (int i = 0; i < n_words; ++i) {
    state_machine.AddTransition(AlphabeticEncoder::kStateMachineStartStateId,
                                0, i);
    state_machine.AddTransition(0, 0, i);
  }
  state_machine.AddTransition(0, AlphabeticEncoder::kStateMachineEndStateId,
                              AlphabeticEncoder::kEndCharacterId);
}

int main(int argc, char** argv) {
  static const unsigned kNumberGenerations = 25;

  std::vector<std::string> code;
  BijectiveChecker checker;
  StateMachine state_machine;
  for (unsigned M = 6; M <= 6; ++M) {
    unsigned N_max = CodeGenerator::MaxNumberElemCodes(M);
    for (unsigned N = 2; N <= N_max; ++N) {
      StateMachineOfAllWords(N, state_machine);

      unsigned L_min = CodeGenerator::MinCodeLength(M, N);
      unsigned L_max = CodeGenerator::MaxCodeLength(M, N);
      for (unsigned L = L_min; L <= L_max; ++L) {
        for (unsigned gen = 0; gen < kNumberGenerations; ++gen) {
          CodeGenerator::GenCode(L, M, N, code);

          for (int i = 0; i < N; ++i) {
            std::cout << code[i] << ' ';
          }
          std::cout << std::endl;

          checker.IsBijective(code, state_machine);
        }
      }
    }
  }
}
