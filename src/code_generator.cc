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

  // We need use state machines with exists ways from start state to all states.
  // Track this relations for keep all states are visited.
  // |I| - number of states
  // N - number of characters
  //        | to                                     |
  // | from |          0|          1| ...|      |I|-1|
  // |------|-----------|-----------|----|-----------|
  // |     0| {0/../N-1}| {0/../N-1}| ...| {0/../N-1}|
  // |     1| {0/../N-1}| {0/../N-1}| ...| {0/../N-1}|
  // |   ...|        ...|        ...| ...|        ...|
  // | |I|-1| {0/../N-1}| {0/../N-1}| ...| {0/../N-1}|
  std::vector<int> transitions[n_states][n_states];
  std::vector<int> unused_chars[n_states];

  std::vector<int> visited_states;
  std::vector<bool> state_is_visited(n_states, false);

  // We need use all characters at least once.
  std::vector<bool> char_is_used(n_elem_codes, false);

  std::queue<int> states_from;
  states_from.push(0);
  state_is_visited[0] = true;
  do {
    int state_from_idx = states_from.front();
    states_from.pop();
    for (int char_idx = 0; char_idx < n_elem_codes; ++char_idx) {
      if (rand() % 2) {
        // Use character [char_idx] as transition event
        int state_to_idx = rand() % n_states;
        transitions[state_from_idx][state_to_idx].push_back(char_idx);
        if (!state_is_visited[state_to_idx]) {
          state_is_visited[state_to_idx] = true;
          states_from.push(state_to_idx);
        }
        if (!char_is_used[char_idx]) {
          char_is_used[char_idx] = true;
        }
      } else {
        unused_chars[state_from_idx].push_back(char_idx);
      }
    }
    visited_states.push_back(state_from_idx);
  } while (!states_from.empty());

  // Check states to visited.
  std::vector<int> dublicated_transitions[n_states];
  for (int i = 0; i < n_states; ++i) {
    for (int j = 0; j < n_states; ++j) {
      if (transitions[i][j].size() > 1) {
        dublicated_transitions[i].push_back(j);
      }
    }
  }
  for (int i = 0; i < n_states; ++i) {
    if (!state_is_visited[i]) {
      for (int char_idx = 0; char_idx < n_elem_codes; ++char_idx) {
        unused_chars[i].push_back(char_idx);
      }
      // Founded not visited state. We need add transition from already
      // visited state.
      do {
        std::vector<int>::iterator it = visited_states.begin() +
                                        rand() % visited_states.size();
        int idx = *it;

        // We have three cases:
        // 1) Visited state has unused characters.
        //    We can use it for creating new transition.
        // 2) Visited state has dublicated transitions to some states
        //    (different characters to same state). We can use one character
        //    and redirect transition to unvisited state. This is not breaks
        //    exists visiting (more than two transitions have).
        // 3) Visited state has not unused characters and all transitions
        //    from it are to different states. We need to delete this state
        //    as useless for current procedure.
        bool has_chars = unused_chars[idx].size() != 0;
        bool has_doubles = dublicated_transitions[idx].size() != 0;

        if (has_chars || has_doubles) {
          if (has_chars && ((rand() % 2) || !has_doubles)) {
            // Use unused character.
            std::vector<int>::iterator char_it =
                unused_chars[idx].begin() + rand() % unused_chars[idx].size();
            transitions[idx][i].push_back(*char_it);
            if (!char_is_used[*char_it]) {
              char_is_used[*char_it] = true;
            }
            unused_chars[idx].erase(char_it);
            break;
          } else {
            // Use dublicated transition.
            std::vector<int>::iterator trans_it =
                dublicated_transitions[idx].begin() +
                rand() % dublicated_transitions[idx].size();
            std::vector<int>::iterator char_it =
                transitions[idx][*trans_it].begin() +
                rand() % transitions[idx][*trans_it].size();
            transitions[idx][i].push_back(*char_it);
            transitions[idx][*trans_it].erase(char_it);
            if (transitions[idx][*trans_it].size() == 1) {
              dublicated_transitions[idx].erase(trans_it);
            }
            break;
          }
        } else {
          // This state is useless. Delete it.
          visited_states.erase(it);
        }
      } while (true);
      visited_states.push_back(i);
    }
  }

  // Check usage of all character.
  for (int i = 0; i < n_elem_codes; ++i) {
    if (!char_is_used[i]) {
      std::vector<int> states_with_unused_char;
      for (int j = 0; j < n_states; ++j) {
        for (int k = 0; k < unused_chars[j].size(); ++k)
        if (i == unused_chars[j][k]) {
          states_with_unused_char.push_back(j);
        } else {
          if (i < unused_chars[j][k]) {
            break;
          }
        }
      }
      std::vector<int>::iterator it = states_with_unused_char.begin() +
                                      rand() % states_with_unused_char.size();
      transitions[*it][rand() % n_states].push_back(i);
      for (std::vector<int>::iterator char_it = unused_chars[*it].begin();
           char_it != unused_chars[*it].end(); ++char_it) {
        if (i == *char_it) {
          unused_chars[*it].erase(char_it);
          break;
        }
      }
    }
  }
  for (int i = 0; i < n_states; ++i) {
    for (int j = 0; j < n_states; ++j) {
      for (int k = 0; k < transitions[i][j].size(); ++k) {
        state_machine->AddTransition(i, j, transitions[i][j][k]);
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
