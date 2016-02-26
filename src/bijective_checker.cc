#include "include/bijective_checker.h"

#include <stdio.h>
#include <stdlib.h>

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
  RemoveBottlenecks();

  code_state_machine.InitContexts();

  return !FindTargetLoop(code_state_machine, first_bad_word, second_bad_word);
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

  std::queue<std::vector<Transition*>* > paths;
  for (int i = 0; i < identity_deficit->transitions_from.size(); ++i) {
    std::vector<Transition*>* new_path = new std::vector<Transition*>();
    new_path->push_back(identity_deficit->transitions_from[i]);
    paths.push(new_path);
  }
  while (!paths.empty()) {
    bool target_loop_found = ProcessNextPath(code_state_machine, paths,
                                             first_bad_word, second_bad_word);
    if (target_loop_found) {
      while (!paths.empty()) {
        delete paths.front();
        paths.pop();
      }
      return true;
    }
  }
  return false;
}

bool BijectiveChecker::ProcessNextPath(
                                const StateMachine& code_state_machine,
                                std::queue<std::vector<Transition*>* >& paths,
                                std::vector<int>* first_bad_word,
                                std::vector<int>* second_bad_word) {
  std::vector<Transition*>* path = paths.front();
  paths.pop();

  const int identity_deficit_id = UnsignedDeficitId(0);
  State* deficit = path->back()->to;

  for (int i = 0; i < deficit->transitions_from.size(); ++i) {
    Transition* trans = deficit->transitions_from[i];
    State* to = trans->to;
    if (std::find(path->begin(), path->end(), trans) == path->end()) {
      if (to->id != identity_deficit_id || path->size() > 1) {
        std::vector<Transition*>* new_path 
            = new std::vector<Transition*>(*path);
        new_path->push_back(trans);

        std::vector<int> first_word;
        std::vector<int> second_word;
        CollectWords(*new_path, first_word, second_word);
        bool context_found = code_state_machine.FindContext(first_word, second_word);

        if (context_found) {
          if (to->id == identity_deficit_id) {
            delete new_path;

            if (first_bad_word) *first_bad_word = first_word;
            if (second_bad_word) *second_bad_word = second_word;

            delete path;
            return true;
          } else {
            paths.push(new_path);
          }
        } else {
          delete new_path;
        }
      }
    }
  }
  delete path;
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
