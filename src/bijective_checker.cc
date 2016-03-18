#include "include/bijective_checker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <algorithm>

#include "include/simple_suffix_tree.h"
#include "include/alphabetic_encoder.h"

bool BijectiveChecker::IsBijective(const std::vector<std::string>& code,
                                   StateMachine& code_state_machine,
                                   std::vector<int>* first_bad_word,
                                   std::vector<int>* second_bad_word) {
  Reset();
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
  AlphabeticEncoder::WriteConfigFile("/home/dmitry/config_2.txt", code, code_state_machine);
  AlphabeticEncoder::WriteCodeStateMachine("/home/dmitry/sm.dot", code, code_state_machine);
  WriteDeficitsStateMachine("/home/dmitry/dm.dot");
  // StateMachine* syn_sm = BuildSynonymyStateMachine(code_state_machine);
  bool loop_found = FindSynonymyLoop(code_state_machine);
  // delete syn_sm;
  return !loop_found;
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
      AddAntitropicDeficits(deficit_id, code_tree, deficits_up_to_build);
      AddIsotropicDeficits(deficit_id, code_tree, deficits_up_to_build);
      processed_deficits[u_deficit_id] = true;
    }
  }
}

void BijectiveChecker::AddIsotropicDeficits(
  int deficit_id,
  const CodeTree& code_tree,
  std::queue<int>& deficits_up_to_build) {
  // Alpha = elem_code + beta.
  // Find all elementary codes which are preffixes of alpha.
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  std::string alpha_suffix_str = alpha_suffix->str();
  int alpha_suffix_length = alpha_suffix->length;
  const CodeTreeNode* code_tree_node = code_tree.GetRoot();
  std::vector<ElementaryCode*> found_elem_codes;
  for (int i = 0; i < alpha_suffix_length; ++i) {
    if (alpha_suffix_str[i] == '0') {
      code_tree_node = code_tree_node->GetLeft();
    } else {
      code_tree_node = code_tree_node->GetRight();
    }
    if (code_tree_node == 0) {
      break;
    }
    if (code_tree_node->GetElemCode() != 0) {
      found_elem_codes.push_back(code_tree_node->GetElemCode());
    }
  }

  for (int i = 0; i < found_elem_codes.size(); ++i) {
    int beta_suffix_idx = alpha_suffix->owners[0]->str.length() -
                          alpha_suffix->length +
                          found_elem_codes[i]->str.length();
    Suffix* beta_suffix = alpha_suffix->owners[0]->suffixes[beta_suffix_idx];
    int state_id = (deficit_id < 0 ? -beta_suffix->id : beta_suffix->id);
    deficits_state_machine_->AddTransition(UnsignedDeficitId(deficit_id),
                                           UnsignedDeficitId(state_id),
                                           found_elem_codes[i]->id);
    deficits_up_to_build.push(state_id);
  }
}

void BijectiveChecker::AddAntitropicDeficits(
    int deficit_id,
    const CodeTree& code_tree,
    std::queue<int>& deficits_up_to_build) {
  // Elem_code = alpha + beta.
  // Find all elementary codes with prefix [alpha].
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  CodeTreeNode* alpha_suffix_node = code_tree.Find(alpha_suffix->str());
  std::vector<ElementaryCode*> found_elem_codes;
  if (alpha_suffix_node != 0) {
    std::queue<CodeTreeNode*> nodes;
    nodes.push(alpha_suffix_node);
    while (!nodes.empty()) {
      CodeTreeNode* node = nodes.front();
      nodes.pop();
      ElementaryCode* node_elem_code = node->GetElemCode();
      CodeTreeNode* node_left = node->GetLeft();
      CodeTreeNode* node_right = node->GetRight();
      if (node_elem_code) found_elem_codes.push_back(node_elem_code);
      if (node_left) nodes.push(node_left);
      if (node_right) nodes.push(node_right);
    }
  }

  for (int i = 0; i < found_elem_codes.size(); ++i) {
    // Suffixes ordered from largest to minimal.
    Suffix* beta_suffix = found_elem_codes[i]->suffixes[alpha_suffix->length];

    // Let identity deficit is an isotropic deficit.
    if (beta_suffix->id != 0) {
      int state_id = (deficit_id < 0 ? beta_suffix->id : -beta_suffix->id);
      deficits_state_machine_->AddTransition(UnsignedDeficitId(deficit_id),
                                             UnsignedDeficitId(state_id),
                                             found_elem_codes[i]->id);
      deficits_up_to_build.push(state_id);
    }
  }
}

BijectiveChecker::BijectiveChecker()
  : deficits_state_machine_(0) {
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
}

void BijectiveChecker::WriteDeficitsStateMachine(const std::string& file_path) {
  // Set states names.
  const int n_states = code_suffixes_.size() * 2 - 1;
  std::vector<std::string> states_names(n_states);
  states_names[UnsignedDeficitId(0)] = "\"/\"";

  // First suffix if empty suffix, starts from 1.
  for (int i = 1; i < code_suffixes_.size(); ++i) {
    std::string str = code_suffixes_[i]->str();
    states_names[UnsignedDeficitId(i)] = "\"" + str + "/\"";
    states_names[UnsignedDeficitId(-i)] = "\"/" + str + "\"";
  }

  // Set transitions names.
  std::vector<std::string> events_names(code_.size());
  for (int i = 0; i < code_.size(); ++i) {
    events_names[i] = code_[i]->str;
  }

  deficits_state_machine_->WriteDot(file_path, states_names, events_names);
}

void BijectiveChecker::RemoveBottlenecks() {
  const int n_states = deficits_state_machine_->GetNumberStates();
  std::queue<State*> states;
  states.push(deficits_state_machine_->GetState(UnsignedDeficitId(0)));

  std::vector<bool> achievable(n_states, false);
  while (!states.empty()) {
    State* state = states.front();
    states.pop();
    achievable[state->id] = true;
    for (int i = 0; i < state->transitions_to.size(); ++i) {
      State* state_from = state->transitions_to[i]->from;
      if (!achievable[state_from->id]) {
        states.push(state_from);
      }
    }
  }
  for (int i = 0; i < n_states; ++i) {
    if (!achievable[i]) {
      deficits_state_machine_->DelState(i);
    }
  }
}

bool BijectiveChecker::FindTargetLoop(const StateMachine& code_state_machine,
                                      std::vector<int>* first_bad_word,
                                      std::vector<int>* second_bad_word) {
  State* identity_deficit = deficits_state_machine_
                              ->GetState(UnsignedDeficitId(0));
  const int n_states = deficits_state_machine_->GetNumberStates();

  std::queue<std::vector<Transition*>* > paths;
  std::queue<bool*> visited_states;
  for (int i = 0; i < identity_deficit->transitions_from.size(); ++i) {
    std::vector<Transition*>* new_path = new std::vector<Transition*>();
    new_path->push_back(identity_deficit->transitions_from[i]);
    paths.push(new_path);

    bool* mem = new bool[n_states];
    memset(mem, false, n_states);
    visited_states.push(mem);
  }

  while (!paths.empty()) {
    bool target_loop_found = ProcessNextPath(code_state_machine, paths,
                                             visited_states,
                                             first_bad_word, second_bad_word);
    if (target_loop_found) {
      while (!paths.empty()) {
        delete paths.front();
        delete visited_states.front();
        paths.pop();
        visited_states.pop();
      }
      return true;
    }
  }
  return false;
}

bool BijectiveChecker::ProcessNextPath(
                                const StateMachine& code_state_machine,
                                std::queue<std::vector<Transition*>* >& paths,
                                std::queue<bool*>& visited_states,
                                std::vector<int>* first_bad_word,
                                std::vector<int>* second_bad_word) {
  const int n_states = deficits_state_machine_->GetNumberStates();

  std::vector<Transition*>* path = paths.front();
  paths.pop();
  bool* visits = visited_states.front();
  visited_states.pop();

  const int identity_deficit_id = UnsignedDeficitId(0);
  State* deficit = path->back()->to;

  for (int i = 0; i < deficit->transitions_from.size(); ++i) {
    Transition* trans = deficit->transitions_from[i];
    State* to = trans->to;
    if (!visits[to->id]) {
      if (to->id != identity_deficit_id || path->size() > 1) {
        std::vector<Transition*>* new_path 
            = new std::vector<Transition*>(*path);
        new_path->push_back(trans);

        if (to->id == identity_deficit_id) {
          std::vector<int> first_word;
          std::vector<int> second_word;
          CollectWords(*new_path, first_word, second_word);
          bool has_kernels = false;
          bool context_found = code_state_machine.FindContext(first_word,
                                                              second_word,
                                                              has_kernels);
          if (context_found) {
            delete new_path;

            if (first_bad_word) *first_bad_word = first_word;
            if (second_bad_word) *second_bad_word = second_word;

            delete path;
            return true;
          } else {
            if (has_kernels) {
              // std::cout << "1" << std::endl;
              paths.push(new_path);

              bool* new_visits = new bool[n_states];
              // memset(new_visits, false, n_states);
              memcpy(new_visits, visits, n_states);
              visited_states.push(new_visits);
            } else {
              delete new_path;
            }
          }
        } else {
          paths.push(new_path);

          bool* new_visits = new bool[n_states];
          memcpy(new_visits, visits, n_states);
          new_visits[to->id] = true;
          visited_states.push(new_visits);
        }
      }
    }
  }
  delete path;
  delete visits;
  return false;
}

void BijectiveChecker::CollectWords(const std::vector<Transition*>& path,
                                    std::vector<int>& first_word,
                                    std::vector<int>& second_word) {
  first_word.clear();
  second_word.clear();

  unsigned size = path.size();
  for (int i = 0; i < size; ++i) {
    Transition* trans = path[i];
    if (SignedDeficitId(trans->from->id) >= 0) {
      first_word.push_back(trans->event_id);
    } else {
      second_word.push_back(trans->event_id);
    }
  }
}

StateMachine* BijectiveChecker::BuildSynonymyStateMachine(
      const StateMachine& code_state_machine) {
  const unsigned kNumCodeSmStates = code_state_machine.GetNumberStates();
  const unsigned max_n_states = deficits_state_machine_->GetNumberStates() *
                                kNumCodeSmStates * kNumCodeSmStates;
  StateMachine* synonymy_sm = new StateMachine(max_n_states);
  bool visited_states[max_n_states];
  memset(visited_states, false, max_n_states);

  std::queue<SynonymyState> states;
  SynonymyState syn_state;
  syn_state.deficit = deficits_state_machine_->GetState(UnsignedDeficitId(0));
  syn_state.upper_state = code_state_machine.GetState(0);
  syn_state.lower_state = syn_state.upper_state;
  states.push(syn_state);

  do {
    SynonymyState syn_state = states.front();
    State* deficit = syn_state.deficit;
    const int syn_state_idx = (deficit->id * kNumCodeSmStates +
                               syn_state.upper_state->id) * kNumCodeSmStates +
                               syn_state.lower_state->id;
    visited_states[syn_state_idx] = true;
    states.pop();

    SynonymyState next_syn_state;
    if (SignedDeficitId(deficit->id) >= 0) {  // event: empty/char
      for (int i = 0; i < deficit->transitions_from.size(); ++i) {
        Transition* def_trans = deficit->transitions_from[i];
        const int event = def_trans->event_id;
        Transition* code_trans = syn_state.lower_state->GetTransition(event);
        if (code_trans != 0) {
          const int next_syn_state_idx = (def_trans->to->id * kNumCodeSmStates +
                                          syn_state.upper_state->id) *
                                          kNumCodeSmStates + code_trans->to->id;
          synonymy_sm->AddTransition(syn_state_idx, next_syn_state_idx,
                                     -event - 1);
          if (!visited_states[next_syn_state_idx]) {
            next_syn_state.deficit = def_trans->to;
            next_syn_state.upper_state = syn_state.upper_state;
            next_syn_state.lower_state = code_trans->to;
            states.push(next_syn_state);
          }
        }
      }
    } else {  // event: char/empty
      for (int i = 0; i < deficit->transitions_from.size(); ++i) {
        Transition* def_trans = deficit->transitions_from[i];
        const int event = def_trans->event_id;
        Transition* code_trans = syn_state.upper_state->GetTransition(event);
        if (code_trans != 0) {
          const int next_syn_state_idx = (def_trans->to->id * kNumCodeSmStates +
                                          code_trans->to->id) *
                                          kNumCodeSmStates + 
                                          syn_state.lower_state->id;
          synonymy_sm->AddTransition(syn_state_idx, next_syn_state_idx,
                                     event + 1);
          if (!visited_states[next_syn_state_idx]) {
            next_syn_state.deficit = def_trans->to;
            next_syn_state.lower_state = syn_state.lower_state;
            next_syn_state.upper_state = code_trans->to;
            states.push(next_syn_state);
          }
        }
      }
    }
  } while (!states.empty());
  return synonymy_sm;
}

bool BijectiveChecker::FindSynonymyLoop(const StateMachine& code_state_machine) {
  static const unsigned kNumCodeSmStates = code_state_machine.GetNumberStates();
  static const unsigned kEndSynHash = SynonymyState::Hash(UnsignedDeficitId(0),
                                                          kNumCodeSmStates - 1,
                                                          kNumCodeSmStates - 1,
                                                          kNumCodeSmStates);
  std::queue<std::vector<unsigned>* > visited_states;
  std::queue<std::vector<int>* > sequences;
  std::queue<SynonymyState> syn_states;

  SynonymyState syn_state;
  syn_state.deficit = deficits_state_machine_->GetState(UnsignedDeficitId(0));
  syn_state.upper_state = code_state_machine.GetState(0);
  syn_state.lower_state = syn_state.upper_state;
  syn_states.push(syn_state);
  visited_states.push(new std::vector<unsigned>());
  sequences.push(new std::vector<int>());

  SynonymyState next_syn_state;
  do {
    syn_state = syn_states.front();
    std::vector<unsigned>* visits = visited_states.front();
    std::vector<int>* sequence = sequences.front();
    visits->push_back(syn_state.Hash(kNumCodeSmStates));

    State* deficit = syn_state.deficit;
    if (SignedDeficitId(deficit->id) >= 0) {  // event: empty/char
      for (int i = 0; i < deficit->transitions_from.size(); ++i) {
        Transition* def_trans = deficit->transitions_from[i];
        const int event = def_trans->event_id;

        if (sequence->size() == 1 && sequence->back() == event + 1) continue;

        Transition* code_trans = syn_state.lower_state->GetTransition(event);
        if (code_trans != 0) {
          next_syn_state.deficit = def_trans->to;
          next_syn_state.upper_state = syn_state.upper_state;
          next_syn_state.lower_state = code_trans->to;
          const unsigned hash = next_syn_state.Hash(kNumCodeSmStates);

          if (hash == kEndSynHash && sequence->size() > 2) {
            std::cout << "seq: ";
            for (int j = 0; j < sequence->size(); ++j) {
              std::cout << sequence->operator[](j) << ' ';
            }
            std::cout << -event-1 << std::endl;
            return true;
          }

          if (std::find(visits->begin(), visits->end(), hash) ==
              visits->end()) {
            std::vector<int>* new_sequence = new std::vector<int>(*sequence);
            new_sequence->push_back(-event - 1);
            sequences.push(new_sequence);
            visited_states.push(new std::vector<unsigned>(*visits));
            syn_states.push(next_syn_state);
          }
        }
      }
    } else {  // event: char/empty
      for (int i = 0; i < deficit->transitions_from.size(); ++i) {
        Transition* def_trans = deficit->transitions_from[i];
        const int event = def_trans->event_id;

        if (sequence->size() == 1 && sequence->back() == -event - 1) continue;

        Transition* code_trans = syn_state.upper_state->GetTransition(event);
        if (code_trans != 0) {
          next_syn_state.deficit = def_trans->to;
          next_syn_state.upper_state = code_trans->to;
          next_syn_state.lower_state = syn_state.lower_state;
          const unsigned hash = next_syn_state.Hash(kNumCodeSmStates);

          if (hash == kEndSynHash && sequence->size() > 2) {
            std::cout << "seq: ";
            for (int j = 0; j < sequence->size(); ++j) {
              std::cout << sequence->operator[](j) << ' ';
            }
            std::cout << event+1 << std::endl;
            return true;
          }


          if (std::find(visits->begin(), visits->end(), hash) ==
              visits->end()) {
            std::vector<int>* new_sequence = new std::vector<int>(*sequence);
            new_sequence->push_back(event + 1);
            sequences.push(new_sequence);
            visited_states.push(new std::vector<unsigned>(*visits));
            syn_states.push(next_syn_state);
          }
        }
      }
    }
    syn_states.pop();
    visited_states.pop();
    sequences.pop();
    delete visits;
    delete sequence;
  } while (!syn_states.empty());

  return false;
}

unsigned BijectiveChecker::SynonymyState::Hash(unsigned code_sm_n_states) {
  return (upper_state->id * code_sm_n_states + lower_state->id) *
         code_sm_n_states + deficit->id;
}

unsigned BijectiveChecker::SynonymyState::Hash(unsigned deficit_id,
                                               unsigned upper_state_id,
                                               unsigned lower_state_id,
                                               unsigned code_sm_n_states) {
  return (upper_state_id * code_sm_n_states + lower_state_id) *
         code_sm_n_states + deficit_id;
}