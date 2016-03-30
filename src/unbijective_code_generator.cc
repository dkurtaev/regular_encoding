#include "include/unbijective_code_generator.h"

#include <algorithm>
#include <iostream>

#include "include/structures.h"
#include "include/code_generator.h"

// Generates different decompositions of same sequence.
void UnbijectiveCodeGenerator::GenDelimeters(int seed_length,
                                             std::vector<int>* delimeters) {
  int n_delimiters[2];
  const int max_n_delims = seed_length - 1;

  n_delimiters[0] = rand(0, max_n_delims);
  GenUniqueUnnegatives(max_n_delims - 1, n_delimiters[0], &delimeters[0]);

  if (n_delimiters[0] == 0) {
    n_delimiters[1] = rand(1, max_n_delims);
  } else {
    n_delimiters[1] = rand(0, n_delimiters[0] == max_n_delims ?
                              max_n_delims - 1 :
                              max_n_delims);
  }

  const int n_delims = n_delimiters[0];
  if (n_delimiters[0] != n_delimiters[1]) {
    GenUniqueUnnegatives(max_n_delims - 1, n_delimiters[1], &delimeters[1]);
  } else {
    // Collect unselected delimeters position.
    int unselected_delims[max_n_delims - n_delims];
    int idx = 0;
    for (int i = 0; i < delimeters[0][0]; ++i) {
      unselected_delims[idx++] = i;
    }
    for (int i = 1; i < n_delims; ++i) {
      const int from = delimeters[0][i - 1];
      const int to = delimeters[0][i];
      for (int j = from + 1; j < to; ++j) {
        unselected_delims[idx++] = j;
      }
    }
    for (int i = delimeters[0][n_delims - 1] + 1; i < max_n_delims; ++i) {
      unselected_delims[idx++] = i;
    }

    // Set one of new delimeter to unselected position.
    // This gives different decompositions.
    const int delim = unselected_delims[rand() % (max_n_delims - n_delims)];
    delimeters[1].push_back(delim);

    std::vector<int> buf;

    // 0 <= L <= delim
    // 0 <= N - 1 - L <= max_n - delim - 1

    // 0 <= L <= delim
    // N + delim - max_n <= L <= N - 1
    const int n_left_delims = rand(
                               std::max(0, n_delims + delim - max_n_delims),
                               std::min(delim, n_delims - 1));
    if (n_left_delims > 0) {
      GenUniqueUnnegatives(delim - 1, n_left_delims, &buf);
      InsertFront(buf, &delimeters[1]);
    }
    const int n_right_delims = n_delims - 1 - n_left_delims;
    if (n_right_delims > 0) {
      GenUniqueUnnegatives(max_n_delims - delim - 1, n_right_delims, &buf);
      InsertBack(&delimeters[1], buf);
    }
  }
}

void UnbijectiveCodeGenerator::ExtractElemCodes(
                                         const std::string& seed,
                                         std::vector<int>* delimeters,
                                         std::vector<int>* codes,
                                         std::vector<std::string>* elem_codes) {
  elem_codes->clear();

  std::vector<std::pair<int, int> > nodes(1, std::pair<int, int>(-1, -1));
  std::vector<int> elem_codes_ids(1, -1);

  int to;
  int node_id;
  int available_elem_code_id = 0;
  for (int i = 0; i < 2; ++i) {
    int from = 0;
    codes[i].clear();
    delimeters[i].push_back(seed.length() - 1);
    
    for (int j = 0; j < delimeters[i].size(); ++j) {
      to = delimeters[i][j];
      node_id = 0;
      std::string elem_code = "";
      for (int k = from; k <= to; ++k) {
        elem_code += seed[k];
        int child_id = (seed[k] == '0' ? nodes[node_id].first :
                                         nodes[node_id].second);
        if (child_id == -1) {
          child_id = nodes.size();

          if (seed[k] == '0') nodes[node_id].first = child_id;
          else nodes[node_id].second = child_id;

          nodes.push_back(std::pair<int, int>(-1, -1));
          elem_codes_ids.push_back(-1);
        }
        node_id = child_id;
      }
      if (elem_codes_ids[node_id] == -1) {
        elem_codes_ids[node_id] = available_elem_code_id;
        available_elem_code_id += 1;
        elem_codes->push_back(elem_code);
      }
      codes[i].push_back(elem_codes_ids[node_id]);
      from = to + 1;
    }
  }
}

void UnbijectiveCodeGenerator::GenStateMachine(int n_elem_codes,
                                               const std::vector<int>* codes,
                                               StateMachine* state_machine) {
  static const int kMinNumberStates = 1;
  static const int kMaxNumberStates = 5;

  const int n_states = rand(kMinNumberStates, kMaxNumberStates);

  do {
    CodeGenerator::GenStateMachine(n_elem_codes, n_states, state_machine);
  } while(!state_machine->IsRecognized(codes[0]) ||
          !state_machine->IsRecognized(codes[1]));
}

void UnbijectiveCodeGenerator::Generate(std::vector<std::string>* code,
                                        StateMachine* state_machine) {
  static const int kSeedMinLength = 2;
  static const int kSeedMaxLength = 10;
  // Generate seed.
  std::string seed = "";
  const int seed_length = rand(kSeedMinLength, kSeedMaxLength);
  for (int i = 0; i < seed_length; ++i) {
    seed += rand() % 2 + '0';
  }

  std::vector<int> delimeters[2];
  std::vector<int> elem_codes_ids[2];
  GenDelimeters(seed_length, delimeters);
  ExtractElemCodes(seed, delimeters, elem_codes_ids, code);
  GenStateMachine(code->size(), elem_codes_ids, state_machine);
}