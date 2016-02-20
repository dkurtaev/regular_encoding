#include "include/bijective_checker.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "include/simple_suffix_tree.h"
#include "include/alphabetic_encoder.h"

bool BijectiveChecker::IsBijective(const std::vector<std::string>& code,
                                   const StateMachine& code_state_machine) {
  Reset();

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
  RemoveDeadTransitions(code_state_machine);
  RemoveBottlenecks();

    WriteDeficitsStateMachine("/home/dmitry/dm.dot");
  AlphabeticEncoder::WriteCodeStateMachine("/home/dmitry/sm.dot", code, code_state_machine);

  return !FindTargetLoop(code_state_machine);
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

bool BijectiveChecker::FindTargetLoop(const StateMachine& code_state_machine) {
  const int n_code_sm_trans = code_state_machine.GetNumberTransitions();
  State* code_sm_start = code_state_machine.GetStartState();
  const int code_sm_end_id = code_state_machine.GetEndState()->id;
  const int n_deficits_sm_trans = deficits_state_machine_
                                    ->GetNumberTransitions();
  State* identity_deficit = deficits_state_machine_
                              ->GetState(UnsignedDeficitId(0));

  std::queue<LoopState*> loop_states;

  // Add initial loop state at identity deficit and empty words.
  LoopState* loop_state = new LoopState();
  loop_state->deficits_trace.resize(n_deficits_sm_trans);
  for (int i = 0; i < 2; ++i) {
    loop_state->words_trace[i].resize(n_code_sm_trans);
    loop_state->words_states[i] = code_sm_start;
  }
  loop_state->deficit_state = identity_deficit;
  loop_states.push(loop_state);

  do {
    loop_state = loop_states.front();
    State* deficit_state = loop_state->deficit_state;
    for (int i = 0; i < deficit_state->transitions_from.size(); ++i) {
      bool target_loop_founded =
          ProcessLoopTransition(loop_state,
                                deficit_state->transitions_from[i],
                                loop_states, code_sm_end_id);
      if (target_loop_founded) {
        while (!loop_states.empty()) {
          delete loop_states.front();
          loop_states.pop();
        }
        return true;
      }
    }
    loop_states.pop();
    delete loop_state;
  } while (!loop_states.empty());

  return false;
}

bool BijectiveChecker::ProcessLoopTransition(LoopState* state,
                                             Transition* def_transition,
                                             std::queue<LoopState*>& states,
                                             unsigned end_state_id) {
  int def_id = SignedDeficitId(def_transition->to->id);
  int char_id = def_transition->event_id;
  enum ProcessingResult {TRANSITION_NOT_EXISTS,
                         TRANSITION_EXISTS,
                         FOUND_LOOP};
  ProcessingResult result = TRANSITION_NOT_EXISTS;

  LoopState* transited_state = new LoopState();
  for (int i = 0; i < 2; ++i) {
    transited_state->words_trace[i] = state->words_trace[i];
    transited_state->words[i] = state->words[i];
    transited_state->words_states[i] = state->words_states[i];
  }
  transited_state->deficits_trace = state->deficits_trace;
  transited_state->deficit_state = def_transition->to;

  const int word_id = (SignedDeficitId(state->deficit_state->id) >= 0 ? LOWER :
                                                                        UPPER);
  Transition* word_transition = transited_state->words_states[word_id]
                                               ->GetTransition(char_id);
  if (word_transition != 0) {
    if (!transited_state->words_trace[word_id][word_transition->id] ||
        !transited_state->deficits_trace[def_transition->id]) {
      transited_state->words_states[word_id] = word_transition->to;
      transited_state->words[word_id].push_back(char_id);

      for (int i = 0; i < word_transition->from->transitions_from.size(); ++i) {
        Transition* trans = word_transition->from->transitions_from[i];
        if (trans->to->id == word_transition->to->id) {
          transited_state->words_trace[word_id][trans->id] = true;
        }
      }

      transited_state->deficits_trace[def_transition->id] = true;
      result = (def_id != 0 ? TRANSITION_EXISTS : FOUND_LOOP);
    }
  }

  switch (result) {
    case TRANSITION_NOT_EXISTS: {
      delete transited_state;
      break;
    }
    case FOUND_LOOP: {
      bool nontrivial_loop_found = false;
      if (transited_state->words_states[0]->id == end_state_id &&
          transited_state->words_states[1]->id == end_state_id) {
        if (transited_state->words[0].size() ==
            transited_state->words[1].size()) {
          for (int i = 0; i < transited_state->words[0].size(); ++i) {
            if (transited_state->words[0][i] !=
                transited_state->words[1][i]) {
              nontrivial_loop_found = true;
              break;
            }
          }
        } else {
          nontrivial_loop_found = true;
        }
      }

      if (!nontrivial_loop_found) {
        if (transited_state->words_states[0]->id != 0 ||
            transited_state->words_states[1]->id != 0) {
          states.push(transited_state);
        } else {
          delete transited_state;
        }
      } else {
        states.push(transited_state);
        return true;
      }
      break;
    }
    case TRANSITION_EXISTS: {
      states.push(transited_state);
      break;
    }
    default: break;
  }
  return false;
}

void BijectiveChecker::LogDeficitsBuilding(int state_id_from,
                                           int state_id_to,
                                           ElementaryCode* elem_code) {
  std::string alpha_suffix = code_suffixes_[abs(state_id_from)]->str();
  std::string beta_suffix = code_suffixes_[abs(state_id_to)]->str();
  if (alpha_suffix == "") {
    alpha_suffix = "E";
  }
  if (beta_suffix == "") {
    beta_suffix = "E";
  }
  std::string log_str = "Added transition (";
  log_str += (state_id_from < 0 ? "b[%d]/E, E/%s)=" : "E/b[%d], %s/E)=");
  log_str += (state_id_to < 0 ? "E/%s, E/%s = " : "%s/E, %s/E = ");
  printf(log_str.c_str(), elem_code->id, alpha_suffix.c_str(),
         beta_suffix.c_str(), beta_suffix.c_str());
  if (state_id_from < 0) {
    printf("[%s/%s\n", elem_code->str.c_str(), alpha_suffix.c_str());
  } else {
    printf("[%s/%s\n", alpha_suffix.c_str(), elem_code->str.c_str());
  }
  fflush(stdout);
}

BijectiveChecker::BijectiveChecker()
  : deficits_state_machine_(0) {
}

void BijectiveChecker::Reset() {
  for (int i = 0; i < code_.size(); ++i) {
    delete code_[i];
  }
  code_.clear();
  code_suffixes_.clear();
  delete deficits_state_machine_;
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

void BijectiveChecker::RemoveDeadTransitions(
  const StateMachine& code_state_machine) {
  int n_transitions = deficits_state_machine_->GetNumberTransitions();
  std::vector<bool> is_visited(n_transitions, false);

  std::queue<State*> deficit_states;
  std::queue<State*> code_sm_states;
  deficit_states.push(deficits_state_machine_->GetState(UnsignedDeficitId(0)));
  code_sm_states.push(code_state_machine.GetStartState());

  while(!deficit_states.empty()) {
    State* deficit_state = deficit_states.front();
    State* code_sm_state = code_sm_states.front();
    deficit_states.pop();
    code_sm_states.pop();

    for (int i = 0; i < deficit_state->transitions_from.size(); ++i) {
      Transition* deficit_transition = deficit_state->transitions_from[i];

      // If transition not visited yet.
      if (!is_visited[deficit_transition->id]) {
        int event_id = deficit_transition->event_id;
        Transition* code_sm_transition = code_sm_state->GetTransition(event_id);

        // If exists corresponding transition at code state machine.
        if (code_sm_transition != 0) {
          deficit_states.push(deficit_transition->to);
          code_sm_states.push(code_sm_transition->to);
          is_visited[deficit_transition->id] = true;
        }
      }
    }
  }

  // Remove dead transitions (not visited due code state machine rules).
  for (int i = 0; i < n_transitions; ++i) {
    if (!is_visited[i]) {
      deficits_state_machine_->DelTransition(i);
    }
  }
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
