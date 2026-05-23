#include <map>
#include <queue>
#include <set>
#include "Automata.h"

// Useful for the McNaughton–Yamada–Thompson algorithm
// See also
// https://dmitrysoshnikov.medium.com/building-a-regexp-machine-part-2-finite-automata-nfa-fragments-5a7c5c005ef0
struct NFAFragment {
    int start;
    int accept;
};

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

NFAFragment build_fragment(RegexNode* node, NFA& nfa) {
    // To do
}

void NFA::build(const RegexNode* root) {
    NFA nfa;
    NFAFragment f = build_fragment(root, nfa);
    nfa.start = f.start;
    nfa.accept = f.accept;
    return nfa;
}

void DFA::build(const NFA& nfa) {
    // To do (Louis)
}
