#include <map>
#include <queue>
#include <set>
#include "Automata.h"

int DFA::step(int state, char c) const {
    if (state < 0 || state >= size()) {
        return -1;
    }
    return transitions[state][(unsigned char)c]; // cast so that in 0 to 255
}

bool DFA::accepts(const std::string& text) const {
    int state = start_state;
    for (char c : text) {
        state = step(state, c);
        if (state == -1)
        return false;
    }
    return final_states[state];
}

int DFA::size() const { return transitions.size(); }

void NFA::build(const RegexNode* root) {
}

void DFA::build(const NFA& nfa) {
}
