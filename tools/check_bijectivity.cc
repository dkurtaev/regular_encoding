// This tool uses input file with encoding data
// and check it bijectiivity. Additinonally, saves visualization
// of code's state machine and deficits state machine.
// Flags:
// [-i] Input file with encoding scheme. See format in alphabetic_encoder.cc
// [-o] Output directory to save .dot files with code's state machine, deficits
//      state machine and .log file with conclusion about bijectivity.

#include <string>
#include <iostream>

#include "include/alphabetic_encoder.h"

std::string FindArg(const std::string& flag, int argc, char** argv);

int main(int argc, char** argv) {
  std::string input_file = FindArg("-i", argc, argv);
  std::string outdir = FindArg("-o", argc, argv);

  if (input_file == "") {
    std::cout << "Please, set input file with flag [-i]" << std::endl;
    return 0;
  }

  AlphabeticEncoder encoder(input_file);
  if (outdir != "") {
    std::cout << encoder.CheckBijective() << std::endl;
//    encoder.WriteCodeStateMachine(outdir + "code_state_machine.dot");
//    encoder.WriteDeficitsStateMachine(outdir + "deficits_state_machine.dot");
  }
}

std::string FindArg(const std::string& flag, int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == flag) {
      if (i + 1 < argc) {
        return std::string(argv[i + 1]);
      } else {
        return "";
      }
    }
  }
  return "";
}
