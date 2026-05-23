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

size_t DFA::step(size_t state, char c) const {
    if (state >= states.size()) {
        return INVALID_STATE;
    }
    return states[state].transitions[(unsigned char)c];
}

bool DFA::accepts(const std::string& text) const {
    size_t state = initial_state;
    for (char c : text) {
        state = step(state, c);
        if (state == INVALID_STATE) {
            return false;
        }
    }
    return states[state].accepting;
}

size_t DFA::size() const { return states.size(); }

NFAFragment build_fragment(const RegexNode* node, NFA& nfa) {
    // To do
}

void NFA::build(const RegexNode* root) {
    states.clear();
    NFAFragment f = build_fragment(root, *this);
    initial_state = f.start;
    accepting_state = f.accept;
}

void DFA::build(const NFA& nfa) {
    // To do (Louis)
}
