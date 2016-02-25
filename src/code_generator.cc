#include "include/code_generator.h"

#include <stdlib.h>
#include <math.h>

#include <queue>
#include <iostream>
#include <algorithm>

#include "include/alphabetic_encoder.h"
#include "include/structures.h"

void CodeGenerator::GenCode(int code_length, int max_elem_code_length,
                            int n_elem_codes, std::vector<std::string>& code) {
  std::vector<int> code_lengths;
  GenCodeLengths(code_length, max_elem_code_length, n_elem_codes, code_lengths);

  code.clear();
  std::vector<int> bin_elem_codes;
  for (int i = 0; i < max_elem_code_length; ++i) {
    if (code_lengths[i] != 0) {
      GenUniqueUnnegatives(pow(2, i + 1) - 1, code_lengths[i], &bin_elem_codes);
      for (int j = 0; j < code_lengths[i]; ++j) {
        code.push_back(CodeFromBinary(bin_elem_codes[j], i + 1));
      }
    }
  }
}

void CodeGenerator::GenCodeLengths(int code_length, int max_elem_code_length,
                                   int n_elem_codes,
                                   std::vector<int>& lengths) {
  std::vector<int> not_full(max_elem_code_length);

  // Initialization.
  lengths.resize(max_elem_code_length);
  for (int i = 0; i < max_elem_code_length - 1; ++i) {
    lengths[i] = 0;
    not_full[i] = i + 1;
  }
  lengths[max_elem_code_length - 1] = 1;
  not_full[max_elem_code_length - 1] = max_elem_code_length;

  // Initial distribution.
  int current_code_length = max_elem_code_length;
  for (int i = 1; i < n_elem_codes; ++i) {
    int id = rand() % not_full.size();
    int length = not_full[id];
    ++lengths[length - 1];
    if (lengths[length - 1] == pow(2, length)) {
      not_full.erase(not_full.begin() + id);
    }
    current_code_length += length;
  }

  // Optimization.
  int decremented_length;
  int incremented_length;
  int min_decremented_length;
  int max_decremented_length;
  while (current_code_length != code_length) {
    if (current_code_length > code_length) {
      min_decremented_length = not_full.front() + 1;
      max_decremented_length = max_elem_code_length;
    } else {
      min_decremented_length = 1;
      max_decremented_length = not_full.back() - 1;
    }

    // Select decremented length.
    do {
      decremented_length = rand(min_decremented_length, max_decremented_length);
    } while (lengths[decremented_length - 1] == 0 ||
             decremented_length == max_elem_code_length &&
             lengths[max_elem_code_length - 1] == 1);

    // Find decremented length position at list of not full lengths.
    // Insert if not exists.
    int dec_length_id;
    bool is_exists = false;
    for (dec_length_id = 0; dec_length_id < not_full.size(); ++dec_length_id) {
      if (not_full[dec_length_id] >= decremented_length) {
        if (not_full[dec_length_id] == decremented_length) {
          is_exists = true;
        }
        break;
      }
    }
    if (!is_exists) {
      not_full.insert(not_full.begin() + dec_length_id, decremented_length);
    }

    // Select incremented length.
    int inc_length_id;
    if (current_code_length > code_length) {
      inc_length_id = rand(0, dec_length_id - 1);
    } else {
      inc_length_id = rand(dec_length_id + 1, not_full.size() - 1);
    }
    incremented_length = not_full[inc_length_id];

    --lengths[decremented_length - 1];
    ++lengths[incremented_length - 1];
    if (lengths[incremented_length - 1] == pow(2, incremented_length)) {
      not_full.erase(not_full.begin() + inc_length_id);
    }
    current_code_length += incremented_length - decremented_length;
  }
}

int CodeGenerator::MinCodeLength(int max_elem_code_length, int n_elem_codes) {
  // Let N = n_elem_codes
  //     M = max_elem_code_length
  //     L = code_length
  // 1) Full powed sum
  //    2 + 4 + 8 + ... + 2^x = 2 * (2^x - 1) <= N and
  //    x <= M.
  // 2) L_full = sum_{i=1}^{x}(i * 2^i) = sum_{k=1}^{x}( sum_{i=k}^{x}(2^i) ) =
  //    sum_{k=1}^{x}( 2^k * (2^{x-k+1} - 1) ) = sum_{k=1}^{x}( 2^{x+1} - 2^k) =
  //    x * 2^{x+1} - 2 * (2^x - 1) = 2^{x+1} * (x - 1) + 2.
  // 3) L = L_full + (N - 2 * (2^x - 1)) * (x + 1) = 2^{x+1} * (x - 1) + 2 +
  //        (N - 2^{x+1} + 2) * (x + 1) = 2^{x+1} * (x - 1) + 2 +
  //        (N + 2) * (x + 1) - 2^{x+1} * (x + 1) =
  //        (N + 2) * (x + 1) + 2 - 2^{x+2}
    if (n_elem_codes <= pow(2, max_elem_code_length + 1) - 2) {
      int m = log(n_elem_codes + 1) / log(2);
      return (n_elem_codes + 1) * m - pow(2, m + 1) + max_elem_code_length + 2;
    } else {
      return 0;
    }
}

int CodeGenerator::MaxCodeLength(int max_elem_code_length, int n_elem_codes) {
  // Let N = n_elem_codes
  //     M = max_elem_code_length
  //     L = code_length
  // 2^M * [M] + 2^{M-1} * [M-1] + ... =
  // 1) Full powed sum
  //    2^M + 2^{M-1} + ... + 2^x = 2^x * (2^(M - x + 1) - 1) =
  //    = 2^{M+1} - 2^x <= N and
  //    x >= 1
  // 2) L_full = sum_{i=x}^{M}(i * 2^i) = x * sum_{i=x}^{M}(2^i) +
  //             sum_{k=x+1}^{M}( sum_{i=k}^{M}(2^i) ) = x * (2^{M+1} - 2^x) +
  //             sum_{k=x+1}^{M}( 2^{M+1} - 2^k ) = x * (2^{M+1} - 2^x) +
  //             2^{M+1} * (M - x) - (2^{M+1} - 2^{x+1}) =
  //             2^x * (2 - x) + 2^{M+1} * (M - 1)
  // 3) L = L_full + (x - 1) * (N - 2^{M+1} + 2^x) = 2^x * (2 - x) +
  //        2^{M+1} * (M - 1) + (x - 1) * (N - 2^{M+1} + 2^x) =
  //        2^{M+1} * (M - x) + 2^x + N * (x - 1)
  if (n_elem_codes <= pow(2, max_elem_code_length + 1) - 2) {
    int x = ceil(log(pow(2, max_elem_code_length + 1) - n_elem_codes) / log(2));
    return pow(2, max_elem_code_length + 1) * (max_elem_code_length - x) +
           pow(2, x) + n_elem_codes * (x - 1);
  } else {
    return 0;
  }
}

int CodeGenerator::MaxNumberElemCodes(int max_elem_code_length) {
  return pow(2, max_elem_code_length + 1) - 2;
}

void CodeGenerator::GenUniqueUnnegatives(int upper_value, int number,
                                         std::vector<int>* values) {
  if (number < upper_value / 2) {
    std::vector<int> available_values(upper_value + 1);
    for (int i = 0; i <= upper_value; ++i) {
      available_values[i] = i;
    }
    values->resize(number);
    int idx;
    for (int i = 0; i < number; ++i) {
      idx = rand() % (upper_value + 1 - i);
      (*values)[i] = available_values[idx];
      available_values.erase(available_values.begin() + idx);
    }
  } else {
    values->resize(upper_value + 1);
    for (int i = 0; i <= upper_value; ++i) {
      (*values)[i] = i;
    }
    int idx;
    for (int i = 0; i < upper_value + 1 - number; ++i) {
      idx = rand() % (upper_value + 1 - i);
      values->erase(values->begin() + idx);
    }
  }
}

void CodeGenerator::GenStateMachine(int n_elem_codes, int n_states,
                                    StateMachine* state_machine) {
  state_machine->Clear();
  state_machine->AddStates(n_states);

  std::vector<int> unused_chars[n_states];
  unused_chars[0].resize(n_elem_codes);
  for (int i = 0; i < n_elem_codes; ++i) {
    unused_chars[0][i] = i;
  }
  for (int i = 1; i < n_states; ++i) {
    unused_chars[i].resize(n_elem_codes);
    std::copy(unused_chars[0].begin(), unused_chars[0].end(),
              unused_chars[i].begin());
  }

  // Make all states are visited.
  std::vector<int> visited_states(1, 0);
  std::vector<bool> char_is_used(n_elem_codes, false);
  int n_used_chars = 0;
  for (int i = 1; i < n_states; ++i) {
    std::vector<int>::iterator from_it = visited_states.begin() +
                                         rand() % visited_states.size();
    int from_idx = *from_it;
    std::vector<int>::iterator char_it = unused_chars[from_idx].begin() +
                                         rand() % unused_chars[from_idx].size();
    int char_idx = *char_it;

    state_machine->AddTransition(from_idx, i, char_idx);
    if (!char_is_used[char_idx]) {
      char_is_used[char_idx] = true;
      ++n_used_chars;
    }
    unused_chars[from_idx].erase(char_it);

    if (unused_chars[from_idx].empty()) {
      visited_states.erase(from_it);
    }
    visited_states.push_back(i);
  }

  // Random transitions.
  for (int i = 0; i < visited_states.size(); ++i) {
    const int from_idx = visited_states[i];
    for (int j = 0; j < unused_chars[from_idx].size(); ++j) {
      if (rand() % 2) {
        const int char_idx = unused_chars[from_idx][j];
        state_machine->AddTransition(from_idx, rand() % n_states, char_idx);
        if (!char_is_used[char_idx]) {
          char_is_used[char_idx] = true;
          ++n_used_chars;
        }

        unused_chars[from_idx].erase(unused_chars[from_idx].begin() + j);
        if (unused_chars[from_idx].empty()) {
          visited_states.erase(visited_states.begin() + i);
          --i;
        }
        --j;
      }
    }
  }

  // Make all characters are used.
  if (n_used_chars != n_elem_codes) {
    std::vector<int> candidates[n_elem_codes];
    for (int i = 0; i < visited_states.size(); ++i) {
      const int from_idx = visited_states[i];
      for (int j = 0; j < unused_chars[from_idx].size(); ++j) {
        const int char_idx = unused_chars[from_idx][j];
        candidates[char_idx].push_back(from_idx);
      }
    }

    for (int i = 0; i < n_elem_codes; ++i) {
      if (!char_is_used[i]) {
        int from_idx = candidates[i][rand() % candidates[i].size()];
        state_machine->AddTransition(from_idx, rand() % n_states, i);
      }
    }
  }
}

unsigned CodeGenerator::GetLNSetLimit(unsigned max_elem_code_length,
                                      unsigned n_elem_codes) {
  unsigned log2 = ceil(log(n_elem_codes) / log(2));
  unsigned L_max = MaxCodeLength(max_elem_code_length, n_elem_codes);
  return std::min(n_elem_codes * (log2 + 1) - (1 << log2), L_max);
}

void CodeGenerator::GenPrefixCode(int max_elem_code_length, int n_elem_codes,
                                  std::vector<std::string>& code) {
  code.clear();
  GenLeafs("", n_elem_codes, max_elem_code_length, code);
}

std::string CodeGenerator::CodeFromBinary(unsigned data, unsigned length) {
  std::string elem_code = "";
  for (int i = 0; i < length; ++i) {
    elem_code += ((data >> i) & 1 ? '1' : '0');
  }
  return elem_code;
}

void CodeGenerator::GenLeafs(const std::string& node_code, int n_leafs,
                             int max_depth, std::vector<std::string>& codes) {
  if (max_depth == 0 || n_leafs == 1 && (rand() % 2)) {
    codes.push_back(node_code);
    return;
  }

  max_depth -= 1;
  int n_left_leafs = rand(std::max(0, n_leafs - (1 << max_depth)),
                          std::min(n_leafs, 1 << max_depth));
  if (n_left_leafs != 0) {
    GenLeafs(node_code + '0', n_left_leafs, max_depth, codes);
  }

  int n_right_leafs = n_leafs - n_left_leafs;
  if (n_right_leafs != 0) {
    GenLeafs(node_code + '1', n_right_leafs, max_depth, codes);
  }

  if (n_left_leafs == 0 && n_right_leafs == 0) {
    codes.push_back(node_code);
  }
}
