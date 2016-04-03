#include "include/bijective_checker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <algorithm>
#include <sstream>

#include "include/simple_suffix_tree.h"
#include "include/alphabetic_encoder.h"

bool BijectiveChecker::IsBijective(const std::vector<std::string>& code,
                                   const StateMachine& code_state_machine,
                                   std::vector<int>* first_bad_word,
                                   std::vector<int>* second_bad_word) {
  Reset();
  code_state_machine_ = &code_state_machine;

  if (first_bad_word) first_bad_word->clear();
  if (second_bad_word) second_bad_word->clear();

  code_.resize(code.size());
  for (int i = 0; i < code.size(); ++i) {
    code_[i] = new ElementaryCode(i, code[i]);
  }

  // Select all suffixes.
  SimpleSuffixTree sst;
  sst.Build(&code_);
  sst.GetSuffixes(&code_suffixes_);  // Includes empty suffix.

  // Build code tree.
  CodeTree code_tree(code_);

  BuildDeficitsStateMachine(code_tree);
  BuildSynonymyStateMachine();

  return !FindSynonymyLoop(first_bad_word, second_bad_word);
}

BijectiveChecker::~BijectiveChecker() {
  Reset();
}

unsigned BijectiveChecker::UnsignedDeficitId(int id) {
  return id + code_suffixes_.size() - 1;
}

int BijectiveChecker::SignedDeficitId(unsigned id) {
  return id - code_suffixes_.size() + 1;
}

void BijectiveChecker::BuildDeficitsStateMachine(const CodeTree& code_tree) {
  // Let 0 state idx - identity deficit,
  //   i<0 state idx - lower deficit lambda/alpha,
  //                   where lambda is empty word,
  //                   alpha - suffix with index |i|
  //   i>0 state idx - upper deficit alpha/lambda,
  //                   alpha index is |i|
  const int n_deficits = code_suffixes_.size() * 2 - 1;
  deficits_state_machine_ = new StateMachine(n_deficits);
  const int identity_deficit_id = UnsignedDeficitId(0);

  // Build deficits machine.
  std::queue<int> deficits_up_to_build;
  for (int i = 0; i < code_.size(); ++i) {
    // Suffixes in descending order:
    // for elementary code 01011
    // [0]: 01011
    // [1]: 1011
    // ...
    // [4]: 1
    // [5]: empty suffix
    int deficit_id = -code_[i]->suffixes[0]->id;

    deficits_state_machine_->AddTransition(identity_deficit_id,
                                           UnsignedDeficitId(deficit_id), i);
    deficits_up_to_build.push(deficit_id);
  }

  std::vector<bool> processed_deficits(n_deficits, false);

  // Identity deficit already processed.
  processed_deficits[identity_deficit_id] = true;

  // Inductive building.
  while (!deficits_up_to_build.empty()) {
    const int deficit_id = deficits_up_to_build.front();
    const unsigned u_deficit_id = UnsignedDeficitId(deficit_id);
    deficits_up_to_build.pop();
    if (!processed_deficits[u_deficit_id]) {
      AddAntitropicDeficits(deficit_id, code_tree, &deficits_up_to_build);
      AddIsotropicDeficits(deficit_id, code_tree, &deficits_up_to_build);
      processed_deficits[u_deficit_id] = true;
    }
  }
}

void BijectiveChecker::AddIsotropicDeficits(
  int deficit_id,
  const CodeTree& code_tree,
  std::queue<int>* deficits_up_to_build) {
  // Alpha = elem_code + beta.
  // Find all elementary codes which are preffixes of alpha.
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  std::vector<ElementaryCode*> upper_elem_codes;
  code_tree.Find(alpha_suffix->str(), &upper_elem_codes);

  const unsigned size = upper_elem_codes.size();
  for (unsigned i = 0; i < size; ++i) {
    int beta_suffix_idx = alpha_suffix->owners[0]->str.length() -
                          alpha_suffix->length +
                          upper_elem_codes[i]->str.length();
    Suffix* beta_suffix = alpha_suffix->owners[0]->suffixes[beta_suffix_idx];
    int state_id = (deficit_id < 0 ? -beta_suffix->id : beta_suffix->id);
    deficits_state_machine_->AddTransition(UnsignedDeficitId(deficit_id),
                                           UnsignedDeficitId(state_id),
                                           upper_elem_codes[i]->id);
    deficits_up_to_build->push(state_id);
  }
}

void BijectiveChecker::AddAntitropicDeficits(
    int deficit_id,
    const CodeTree& code_tree,
    std::queue<int>* deficits_up_to_build) {
  // Elem_code = alpha + beta.
  // Find all elementary codes with prefix [alpha].
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  CodeTreeNode* alpha_suffix_node = code_tree.Find(alpha_suffix->str());
  if (alpha_suffix_node) {
    std::vector<ElementaryCode*> lower_elem_codes;
    alpha_suffix_node->GetLowerElemCodes(&lower_elem_codes);

    const unsigned size = lower_elem_codes.size();
    for (int i = 0; i < size; ++i) {
      // Suffixes ordered from largest to minimal.
      Suffix* beta_suffix = lower_elem_codes[i]->suffixes[alpha_suffix->length];

      // Let identity deficit is an isotropic deficit.
      if (beta_suffix->id != 0) {
        int state_id = (deficit_id < 0 ? beta_suffix->id : -beta_suffix->id);
        deficits_state_machine_->AddTransition(UnsignedDeficitId(deficit_id),
                                               UnsignedDeficitId(state_id),
                                               lower_elem_codes[i]->id);
        deficits_up_to_build->push(state_id);
      }
    }
  }
}

BijectiveChecker::BijectiveChecker()
  : deficits_state_machine_(0),
    synonymy_state_machine_(0) {
}

void BijectiveChecker::Reset() {
  for (int i = 0; i < code_.size(); ++i) {
    delete code_[i];
  }
  code_.clear();

  for (int i = 0; i < code_suffixes_.size(); ++i) {
    delete code_suffixes_[i];
  }
  code_suffixes_.clear();

  delete deficits_state_machine_;
  deficits_state_machine_ = 0;
  delete synonymy_state_machine_;
  synonymy_state_machine_ = 0;
  code_state_machine_ = 0;
}

void BijectiveChecker::WriteDeficitsStateMachine(const std::string& file_path) {
  // Set states names.
  const int n_states = deficits_state_machine_->GetNumberStates();
  std::vector<std::string> states_names(n_states);
  states_names[UnsignedDeficitId(0)] = "\"\u03bb/\u03bb\"";

  // First suffix if empty suffix, starts from 1.
  for (int i = 1; i < code_suffixes_.size(); ++i) {
    std::string str = code_suffixes_[i]->str();
    states_names[UnsignedDeficitId(i)] = "\"" + str + "/\u03bb\"";
    states_names[UnsignedDeficitId(-i)] = "\"\u03bb/" + str + "\"";
  }

  // Set transitions names.
  std::map<int, std::string> events_names;
  const int n_codes = code_.size();
  for (int i = 0; i < n_codes; ++i) {
    events_names[i] = code_[i]->str;
  }
  deficits_state_machine_->WriteDot(file_path, states_names, events_names);
}

void BijectiveChecker::WriteSynonymyStateMachine(const std::string& file_path) {
  const unsigned kNumCodeSmStates = code_state_machine_->GetNumberStates();
  const unsigned kNumDefsSmStates = deficits_state_machine_->GetNumberStates();
  const unsigned kNumSuffixes = code_suffixes_.size();

  // Deficits names.
  std::vector<std::string> deficits_names(kNumDefsSmStates);
  deficits_names[UnsignedDeficitId(0)] = "\u03bb/\u03bb";
  for (int i = 1; i < kNumSuffixes; ++i) {
    std::string str = code_suffixes_[i]->str();
    deficits_names[UnsignedDeficitId(i)] = str + "/\u03bb";
    deficits_names[UnsignedDeficitId(-i)] = "\u03bb/" + str;
  }

  // Code state machine states names.
  std::vector<std::string> code_sm_states_names(kNumCodeSmStates);
  code_sm_states_names[0] = "st";
  for (int i = 1; i < kNumCodeSmStates - 1; ++i) {
    std::ostringstream ss;
    ss << (i + 1);
    code_sm_states_names[i] = ss.str();
  }
  code_sm_states_names[kNumCodeSmStates - 1] = "end";

  std::vector<std::string> states_names(kNumDefsSmStates * kNumCodeSmStates *
                                        kNumCodeSmStates);
  unsigned idx = 0;
  for (unsigned i = 0; i < kNumDefsSmStates; ++i) {
    for (unsigned j = 0; j < kNumCodeSmStates; ++j) {
      for (unsigned k = 0; k < kNumCodeSmStates; ++k) {
        states_names[idx++] = "\"(" + deficits_names[i] + ", " +
                              code_sm_states_names[j] + "/" +
                              code_sm_states_names[k] + ")\"";
      }
    }
  }

  // Set transitions names.
  std::map<int, std::string> events_names;
  const int n_codes = code_.size();
  for (int i = 0; i < n_codes; ++i) {
    events_names[i + 1] = code_[i]->str + "/\u03bb";
    events_names[-i - 1] = "\u03bb/" + code_[i]->str;
  }

  synonymy_state_machine_->WriteDot(file_path, states_names, events_names);
}

void BijectiveChecker::BuildSynonymyStateMachine() {
  const unsigned kNumCodeSmStates = code_state_machine_->GetNumberStates();
  const unsigned kNumDefsSmStates = deficits_state_machine_->GetNumberStates();
  const unsigned kNumSynonymyStates = kNumCodeSmStates * kNumCodeSmStates *
                                      kNumDefsSmStates;

  synonymy_state_machine_ = new StateMachine(kNumSynonymyStates);

  bool state_is_processed[kNumSynonymyStates];
  memset(state_is_processed, false, kNumSynonymyStates);

  SynonymyState syn_state;
  syn_state.deficit = deficits_state_machine_->GetState(UnsignedDeficitId(0));
  syn_state.upper_state = code_state_machine_->GetState(0);
  syn_state.lower_state = syn_state.upper_state;
  state_is_processed[syn_state.Hash(kNumCodeSmStates)] = true;

  std::queue<SynonymyState> syn_states;
  syn_states.push(syn_state);

  SynonymyState next_syn_state;
  do {
    syn_state = syn_states.front();
    syn_states.pop();

    const unsigned hash_from = syn_state.Hash(kNumCodeSmStates);

    State* deficit = syn_state.deficit;
    if (SignedDeficitId(deficit->id) >= 0) {  // event: empty/char
      for (int i = 0; i < deficit->transitions.size(); ++i) {
        Transition* def_trans = deficit->transitions[i];
        const int event = def_trans->event_id;

        Transition* code_trans = syn_state.lower_state->GetTransition(event);
        if (code_trans != 0) {
          next_syn_state.deficit = def_trans->to;
          next_syn_state.upper_state = syn_state.upper_state;
          next_syn_state.lower_state = code_trans->to;
          const unsigned hash_to = next_syn_state.Hash(kNumCodeSmStates);
          synonymy_state_machine_->AddTransition(hash_from, hash_to,
                                                 -event - 1);
          if (!state_is_processed[hash_to]) {
            syn_states.push(next_syn_state);
            state_is_processed[hash_to] = true;
          }
        }
      }
    } else {  // event: char/empty
      for (int i = 0; i < deficit->transitions.size(); ++i) {
        Transition* def_trans = deficit->transitions[i];
        const int event = def_trans->event_id;

        Transition* code_trans = syn_state.upper_state->GetTransition(event);
        if (code_trans != 0) {
          next_syn_state.deficit = def_trans->to;
          next_syn_state.upper_state = code_trans->to;
          next_syn_state.lower_state = syn_state.lower_state;
          const unsigned hash_to = next_syn_state.Hash(kNumCodeSmStates);
          synonymy_state_machine_->AddTransition(hash_from, hash_to, event + 1);
          if (!state_is_processed[hash_to]) {
            syn_states.push(next_syn_state);
            state_is_processed[hash_to] = true;
          }
        }
      }
    }
  } while (!syn_states.empty());
}

bool BijectiveChecker::FindSynonymyLoop(std::vector<int>* first_bad_word,
                                        std::vector<int>* second_bad_word) {
  // Exists two types of paths: trivial and not-trivial.
  // Trivial paths collects same words. Corresponding sequence is
  // (a)(-a)(b)(-b)...(c)(-c)
  // If new symbol breaks this structure, path in not trivial.
  //
  // Trivial loops need to be simple (without self intersections by states).
  // Not trivial loops tracks by shared states visiting log (use only unvisited
  // states).
  const unsigned kNumSynonymyStates = synonymy_state_machine_
                                      ->GetNumberStates();
  const unsigned kNumCodeSmStates = code_state_machine_->GetNumberStates();
  const unsigned kEndSynHash = kNumCodeSmStates * kNumCodeSmStates *
                               (UnsignedDeficitId(0) + 1) - 1;
  const unsigned kStartSynHash = UnsignedDeficitId(0) * kNumCodeSmStates *
                                 kNumCodeSmStates;

  bool state_is_visited[kNumSynonymyStates];  // Used for non-trivial sequences.
  memset(state_is_visited, false, kNumSynonymyStates);

  std::queue<bool> flags_of_triviality;
  std::queue<Transition**> paths;

  // Starts from initial state.
  State* state = synonymy_state_machine_->GetState(kStartSynHash);
  Transition** path = 0;
  unsigned n_trans = state->transitions.size();
  for (unsigned i = 0; i < n_trans; ++i) {
    path = new Transition*[1];
    path[0] = state->transitions[i];
    paths.push(path);
    flags_of_triviality.push(true);
  }

  unsigned suqence_length = 1;
  while (!paths.empty()) {
    // Process all paths with current length.
    const unsigned n_paths = paths.size();

    for (unsigned i = 0; i < n_paths; ++i) {
      path = paths.front();
      const bool sequence_is_trivial = flags_of_triviality.front();
      paths.pop();
      flags_of_triviality.pop();

      Transition* trans = path[suqence_length - 1];
      int last_char = trans->event_id;
      state = trans->to;
      n_trans = state->transitions.size();

      // Process each transition of current state.
      for (unsigned j = 0; j < n_trans; ++j) {
        trans = state->transitions[j];
        bool new_sequence_is_trivial = sequence_is_trivial;

        // Check next state to unvisiting.
        if (sequence_is_trivial) {
          // Skip new transition if next state already visited by trivial
          // parent path or other untrivials paths.
          // bool use_new_path = !state_is_visited[trans->to->id];
          bool use_new_path = true;
          for (unsigned k = 0; use_new_path && k < suqence_length; ++k) {
            State* state_from = path[k]->from;
            if (state_from == trans->to) {
              use_new_path = false;
            }
          }

          if (use_new_path) {
            // Check triviality of new path.
            if (suqence_length % 2 == 1 && last_char + trans->event_id != 0) {
              new_sequence_is_trivial = false;
            }
          } else {
            continue;  // Skip this transition.
          }
        }

        const int to_id = trans->to->id;
        if (to_id != kEndSynHash || new_sequence_is_trivial) {
          if (new_sequence_is_trivial || !state_is_visited[to_id] ||
              to_id == kStartSynHash) {
            if (!new_sequence_is_trivial) {
              state_is_visited[to_id] = true;
            }
            Transition** new_path = new Transition*[suqence_length + 1];
            memcpy(new_path, path, sizeof(Transition*) * suqence_length);
            new_path[suqence_length] = trans;

            paths.push(new_path);
            flags_of_triviality.push(new_sequence_is_trivial);
          }
        } else {
          // Extract not bijective words.
          if (first_bad_word != 0 && second_bad_word != 0) {
            int symbol;
            for (unsigned k = 0; k < suqence_length; ++k) {
              symbol = path[k]->event_id;
              if (symbol > 0)
                first_bad_word->push_back(symbol - 1);
              else
                second_bad_word->push_back(-symbol - 1);
            }
            symbol = trans->event_id;
            if (symbol > 0)
              first_bad_word->push_back(symbol - 1);
            else
              second_bad_word->push_back(-symbol - 1);
          }
          while (!paths.empty()) {
            delete[] paths.front();
            paths.pop();
          }
          return true;
        }
      }
      delete[] path;
    }
    ++suqence_length;
  }

  return false;
}

unsigned BijectiveChecker::SynonymyState::Hash(unsigned code_sm_n_states) {
  return (deficit->id * code_sm_n_states + upper_state->id) *
         code_sm_n_states + lower_state->id;
}
