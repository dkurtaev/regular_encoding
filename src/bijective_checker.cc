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
  const unsigned kStartDefId = UnsignedDeficitId(0);
  const unsigned kNumDefSmStates = deficits_state_machine_->GetNumberStates();
  const unsigned kNumCodeSmStates = code_state_machine_->GetNumberStates();
  const unsigned kStartSynHash =
      SynonymyState::Hash(kStartDefId, 0, 0, kNumCodeSmStates);
  const unsigned kEndSynHash =
      SynonymyState::Hash(kStartDefId, kNumCodeSmStates - 1,
                          kNumCodeSmStates - 1, kNumCodeSmStates);
  const unsigned kMaxNumSynStates = kNumDefSmStates * kNumCodeSmStates *
                                    kNumCodeSmStates;

  // Three visiting states for each state:
  enum VisitingState { FREE, FREE_FOR_NONTRIVIAL, BUSY };
  VisitingState states_visiting[kMaxNumSynStates];
  memset(states_visiting, FREE, sizeof(VisitingState) * kMaxNumSynStates);

  SynonymyState syn_state;
  SynonymyState next_syn_state;
  std::queue<SynonymyState> states;

  // Fill single character sequences.
  State* deficit  = deficits_state_machine_->GetState(kStartDefId);
  syn_state.upper_state = code_state_machine_->GetState(0);
  syn_state.is_tivial = true;
  unsigned size = deficit->transitions.size();
  for (unsigned i = 0; i < size; ++i) {
    Transition* trans = deficit->transitions[i];
    Transition* code_sm_trans = syn_state.upper_state
                                ->GetTransition(trans->event_id);
    if (code_sm_trans != 0) {
      syn_state.deficit = trans->to;
      syn_state.lower_state = code_sm_trans->to;
      syn_state.sequence = new int[1];
      syn_state.sequence[0] = -trans->event_id - 1;
      states.push(syn_state);
    }
  }

  unsigned sequnce_length = 1;
  while (!states.empty()) {
    size = states.size();
    for (unsigned i = 0; i < size; ++i) {
      syn_state = states.front();
      deficit = syn_state.deficit;
      const unsigned n_trans = deficit->transitions.size();
      for (int j = 0; j < n_trans; ++j) {
        Transition* trans = deficit->transitions[j];
        const int event = trans->event_id;

        // Check next state to existing.
        next_syn_state = syn_state;
        next_syn_state.deficit = trans->to;
        trans = (SignedDeficitId(deficit->id) >= 0 ?
                   syn_state.lower_state->GetTransition(event) :
                   syn_state.upper_state->GetTransition(event));
        int new_char;

        if (trans != 0) {
          if (SignedDeficitId(deficit->id) >= 0) {
            next_syn_state.lower_state = trans->to;
            new_char = -event - 1;
          } else {
            next_syn_state.upper_state = trans->to;
            new_char = event + 1;
          }
        }
        else {
          continue;  // Transition not exists.
        }

        // Check next state to unvisiting.
        const unsigned to_hash = next_syn_state.Hash(kNumCodeSmStates);
        VisitingState vis_state = states_visiting[to_hash];
        if (syn_state.is_tivial) {
          if (vis_state == FREE) {
            states_visiting[to_hash] = FREE_FOR_NONTRIVIAL;

            // Check triviality of new path.
            if (sequnce_length % 2 == 1 &&
                syn_state.sequence[sequnce_length - 1] + new_char != 0) {
              next_syn_state.is_tivial = false;
            }
          } else {
            continue;  // Skip this transition.
          }
        }

        if (to_hash != kEndSynHash || next_syn_state.is_tivial) {
          vis_state = states_visiting[to_hash];
          if (next_syn_state.is_tivial || vis_state == FREE ||
              vis_state == FREE_FOR_NONTRIVIAL) {
            if (!next_syn_state.is_tivial) {
              states_visiting[to_hash] = BUSY;
            }
            int* new_sequence = new int[sequnce_length + 1];
            memcpy(new_sequence, next_syn_state.sequence,
                   sizeof(int) * sequnce_length);
            new_sequence[sequnce_length] = new_char;
            next_syn_state.sequence = new_sequence;

            states.push(next_syn_state);
          }
        } else {
          // Extract not bijective words.
          if (first_bad_word != 0 && second_bad_word != 0) {
            int symbol;
            for (unsigned k = 0; k < sequnce_length; ++k) {
              symbol = next_syn_state.sequence[k];
              if (symbol > 0) {
                first_bad_word->push_back(symbol - 1);
              }
              else {
                second_bad_word->push_back(-symbol - 1);
              }
            }
            if (new_char > 0) {
              first_bad_word->push_back(new_char - 1);
            }
            else {
              second_bad_word->push_back(-new_char - 1);
            }
          }
          while (!states.empty()) {
            delete[] states.front().sequence;
            states.pop();
          }
          return true;
        }
      }

      delete[] syn_state.sequence;
      states.pop();
    }
    ++sequnce_length;
  }
  return false;
}

unsigned BijectiveChecker::SynonymyState::Hash(unsigned n_code_sm_states) {
  return SynonymyState::Hash(deficit->id, upper_state->id, lower_state->id,
                             n_code_sm_states);
}

unsigned BijectiveChecker::SynonymyState::Hash(
    unsigned deficit_id, unsigned upper_state_id, unsigned lower_state_id,
    unsigned n_code_sm_states) {
  return (deficit_id * n_code_sm_states + upper_state_id) * n_code_sm_states +
      lower_state_id;
}
