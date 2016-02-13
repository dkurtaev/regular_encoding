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
                                     // Suffixies in increasing order.

  // Build code tree.
  code_tree_ = new CodeTree(code_);

  BuildDeficitsStateMachine();

  this->WriteDeficitsStateMachine("/home/dmitry/tmp_full.dot");

  RemoveDeadTransitions(code_state_machine);

  this->WriteDeficitsStateMachine("/home/dmitry/tmp.dot");

  RemoveBottlenecks();

  this->WriteDeficitsStateMachine("/home/dmitry/tmp_short.dot");

  bool target_loop_founded = FindTargetLoop(code_state_machine);

  return !target_loop_founded;
}

void BijectiveChecker::BuildDeficitsStateMachine() {
  // Let 0 state idx - identity deficit,
  //   i<0 state idx - lower deficit lambda/alpha,
  //                   where lambda is empty word,
  //                   alpha - suffix with index |i|
  //   i>0 state idx - upper deficit alpha/lambda,
  //                   alpha index is |i|
  deficits_state_machine_ = new StateMachine();
  deficits_state_machine_->AddState(0);
  deficits_state_machine_->SetStartState(0);

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
    int state_id = -code_[i]->suffixes[0]->id;
    deficits_state_machine_->AddState(state_id);
    deficits_state_machine_->AddTransition(0, state_id, i);
    deficits_up_to_build.push(state_id);

    // printf("Added transition (E/b[%d], E/E)=E/%s\n",
    //        i, code_[i]->str.c_str());
    // fflush(stdout);
  }

  // Tracking processed deficits.
  // ... -3 -2 -1 0 1 2 3 ...
  // Where -i: deficit lambda/alpha[i], 
  //       +i: deficit alpha[i]/lambda, 
  //        0: identity deficit lambda/lambda.
  std::vector<bool> processed_deficits(code_suffixes_.size() * 2 - 1, false);

  // Identity deficit already processed.
  processed_deficits[code_suffixes_.size() - 1] = true;

  // Inductive building.
  while (!deficits_up_to_build.empty()) {
    int deficit_id = deficits_up_to_build.front();
    deficits_up_to_build.pop();
    if (!processed_deficits[deficit_id + code_suffixes_.size() - 1]) {
      AddAntitropicDeficits(deficit_id, deficits_up_to_build);
      AddIsotropicDeficits(deficit_id, deficits_up_to_build);
      processed_deficits[deficit_id + code_suffixes_.size() - 1] = true;
    }
  }
}

void BijectiveChecker::AddIsotropicDeficits(
  int deficit_id,
  std::queue<int>& deficits_up_to_build) {
  // Alpha = elem_code + beta.
  // Find all elementary codes which are preffixes of alpha.
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  std::string alpha_suffix_str = alpha_suffix->str();
  int alpha_suffix_length = alpha_suffix->length;
  CodeTreeNode* code_tree_node = code_tree_->GetRoot();
  std::vector<ElementaryCode*> founded_elem_codes;
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
      founded_elem_codes.push_back(code_tree_node->GetElemCode());
    }
  }

  for (int i = 0; i < founded_elem_codes.size(); ++i) {
    int beta_suffix_idx = alpha_suffix->owners[0]->str.length() -
                          alpha_suffix->length +
                          founded_elem_codes[i]->str.length();
    Suffix* beta_suffix = alpha_suffix->owners[0]->suffixes[beta_suffix_idx];
    int state_id = (deficit_id < 0 ? -beta_suffix->id : beta_suffix->id);
    deficits_state_machine_->AddState(state_id);
    deficits_state_machine_->AddTransition(deficit_id, state_id,
                                           founded_elem_codes[i]->id);
    deficits_up_to_build.push(state_id);
    // LogDeficitsBuilding(deficit_id, state_id, founded_elem_codes[i]);
  }
}

void BijectiveChecker::AddAntitropicDeficits(
    int deficit_id,
    std::queue<int>& deficits_up_to_build) {
  // Elem_code = alpha + beta.
  // Find all elementary codes with prefix [alpha].
  Suffix* alpha_suffix = code_suffixes_[abs(deficit_id)];
  CodeTreeNode* alpha_suffix_node = code_tree_->Find(alpha_suffix->str());
  std::vector<ElementaryCode*> founded_elem_codes;
  if (alpha_suffix_node != 0) {
    std::queue<CodeTreeNode*> nodes;
    nodes.push(alpha_suffix_node);
    while (!nodes.empty()) {
      CodeTreeNode* node = nodes.front();
      nodes.pop();
      ElementaryCode* node_elem_code = node->GetElemCode();
      CodeTreeNode* node_left = node->GetLeft();
      CodeTreeNode* node_right = node->GetRight();
      if (node_elem_code)
        founded_elem_codes.push_back(node_elem_code);
      if (node_left)
        nodes.push(node_left);
      if (node_right)
        nodes.push(node_right);
    }
  }

  for (int i = 0; i < founded_elem_codes.size(); ++i) {
    // Suffixes ordered from largest to minimal.
    Suffix* beta_suffix = founded_elem_codes[i]->suffixes[alpha_suffix->length];

    // Let identity deficit is an isotropic deficit.
    if (beta_suffix->id != 0) {
      int state_id = (deficit_id < 0 ? beta_suffix->id : -beta_suffix->id);
      deficits_state_machine_->AddState(state_id);
      deficits_state_machine_->AddTransition(deficit_id,
                                             state_id,
                                             founded_elem_codes[i]->id);
      deficits_up_to_build.push(state_id);
      // LogDeficitsBuilding(deficit_id, state_id, founded_elem_codes[i]);
    }
  }
}

bool BijectiveChecker::FindTargetLoop(const StateMachine& code_state_machine) {
  std::queue<LoopState*> loop_states;

  // Add initial loop state at identity deficit and empty words.
  LoopState* loop_state = new LoopState();
  loop_state->deficits_trace
      .resize(deficits_state_machine_->GetNumberTransitions());
  loop_state->upper_word_trace
      .resize(code_state_machine.GetNumberTransitions());
  loop_state->lower_word_trace
      .resize(code_state_machine.GetNumberTransitions());
  loop_state->upper_word_state = code_state_machine.GetStartState();
  loop_state->lower_word_state = code_state_machine.GetStartState();
  loop_state->deficit_state = deficits_state_machine_->GetStartState();
  loop_states.push(loop_state);

  while (!loop_states.empty()) {
    loop_state = loop_states.front();
    for (int i = 0; i < loop_state->deficit_state->transitions.size(); ++i) {
      bool target_loop_founded =
          ProcessLoopTransition(loop_state,
                                loop_state->deficit_state->transitions[i],
                                &loop_states);
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
  }
  return false;
}

bool BijectiveChecker::ProcessLoopTransition(LoopState* state,
                                             Transition* def_transition,
                                             std::queue<LoopState*>* states) {
  int def_id = def_transition->to->id;
  int char_id = def_transition->event_id;
  enum ProcessingResult {TRANSITION_NOT_EXISTS,
                         TRANSITION_EXISTS,
                         FOUND_LOOP};
  ProcessingResult result = TRANSITION_NOT_EXISTS;
  LoopState* transited_state = new LoopState();
  transited_state->upper_word_trace = state->upper_word_trace;
  transited_state->lower_word_trace = state->lower_word_trace;
  transited_state->deficits_trace = state->deficits_trace;
  transited_state->deficit_state = def_transition->to;
  transited_state->lower_word = state->lower_word;
  transited_state->lower_word_state = state->lower_word_state;
  transited_state->upper_word = state->upper_word;
  transited_state->upper_word_state = state->upper_word_state;

  Transition* word_transition;
  if (state->deficit_state->id >= 0) {
    word_transition = transited_state->lower_word_state->GetTransition(char_id);
    if (word_transition != 0) {
      if (!transited_state->lower_word_trace[word_transition->id] ||
          !transited_state->deficits_trace[def_transition->id]) {
        transited_state->lower_word_state = word_transition->to;
        transited_state->lower_word.push_back(char_id);
        transited_state->lower_word_trace[word_transition->id] = true;
        transited_state->deficits_trace[def_transition->id] = true;
        result = (def_id != 0 ? TRANSITION_EXISTS : FOUND_LOOP);
      }
    }
  } else {
    word_transition = transited_state->upper_word_state->GetTransition(char_id);
    if (word_transition != 0) {
      if (!transited_state->upper_word_trace[word_transition->id] ||
          !transited_state->deficits_trace[def_transition->id]) {
        transited_state->upper_word_state = word_transition->to;
        transited_state->upper_word.push_back(char_id);
        transited_state->upper_word_trace[word_transition->id] = true;
        transited_state->deficits_trace[def_transition->id] = true;
        result = (def_id != 0 ? TRANSITION_EXISTS : FOUND_LOOP);
      }
    }
  }

  switch (result) {
    case TRANSITION_NOT_EXISTS: {
      delete transited_state;
      break;
    }
    case FOUND_LOOP: {
      states->push(transited_state);
      bool nontrivial_loop_founded = false;
      if (transited_state->upper_word.size() ==
          transited_state->lower_word.size()) {
        for (int i = 0; i < transited_state->upper_word.size(); ++i) {
          if (transited_state->upper_word[i] !=
              transited_state->lower_word[i]) {
            nontrivial_loop_founded = true;
          }
        }
      } else {
        nontrivial_loop_founded = true;
      }
      if (nontrivial_loop_founded) {
        // Check for word ending.
        if (transited_state->lower_word_state
            ->GetTransition(AlphabeticEncoder::kEndCharacterId) != 0 &&
            transited_state->upper_word_state
            ->GetTransition(AlphabeticEncoder::kEndCharacterId) != 0) {
          return true;
        }
      }
      break;
    }
    case TRANSITION_EXISTS: {
      states->push(transited_state);
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
  : code_tree_(0),
    deficits_state_machine_(0) {
}

void BijectiveChecker::Reset() {
  for (int i = 0; i < code_.size(); ++i) {
    delete code_[i];
  }
  code_.clear();
  code_suffixes_.clear();
  delete code_tree_;
  delete deficits_state_machine_;
}

void BijectiveChecker::WriteDeficitsStateMachine(
  const std::string& file_path) const {
  // Set states names.
  std::map<int, std::string> states_names;
  states_names[0] = "\"/\"";
  // First suffix if empty suffix, starts from 1.
  for (int i = 1; i < code_suffixes_.size(); ++i) {
    states_names[i] = "\"" + code_suffixes_[i]->str() + "/\"";
    states_names[-i] = "\"/" + code_suffixes_[i]->str() + "\"";
  }

  // Set transitions names.
  std::map<int, std::string> events_names;
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
  deficit_states.push(deficits_state_machine_->GetStartState());
  code_sm_states.push(code_state_machine.GetStartState());

  while(!deficit_states.empty()) {
    State* deficit_state = deficit_states.front();
    State* code_sm_state = code_sm_states.front();
    deficit_states.pop();
    code_sm_states.pop();
    for (int i = 0; i < deficit_state->transitions.size(); ++i) {
      Transition* deficit_transition = deficit_state->transitions[i];

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
  const int n_suffixes = code_suffixes_.size();

  std::queue<State*> states;
  states.push(deficits_state_machine_->GetStartState());

  std::vector<bool> achievable(deficits_state_machine_->GetNumberStates(),
                               false);
  while (!states.empty()) {
    State* state = states.front();
    states.pop();
    achievable[state->id + n_suffixes - 1] = true;
    std::vector<Transition*>::iterator it = state->transitions_to.begin();
    for (it; it != state->transitions_to.end(); ++it) {
      State* state_from = (*it)->from;
      if (!achievable[state_from->id + n_suffixes - 1]) {
        states.push(state_from);
      }
    }
  }
  const int n_states = deficits_state_machine_->GetNumberStates();
  for (int i = 0; i < n_states; ++i) {
    if (!achievable[i]) {
      deficits_state_machine_->DelState(i - n_suffixes + 1);
    }
  }
}
