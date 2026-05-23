#include <map>
#include <queue>
#include <set>
#include "Automata.h"

// Useful for the McNaughton–Yamada–Thompson algorithm
// See also
// https://dmitrysoshnikov.medium.com/building-a-regexp-machine-part-2-finite-automata-nfa-fragments-5a7c5c005ef0
struct NFAFragment {
    size_t start;
    size_t accept;
};

size_t NFA::add_state() {
    states.push_back({});
    return states.size() - 1;
}

void NFA::add_transition(size_t from, char c, size_t to) {
    states[from].transitions.push_back({c, to});
}

void NFA::add_epsilon(size_t from, size_t to) {
    states[from].epsilon.push_back(to);
}

NFAFragment build_fragment(const RegexNode* node, NFA& nfa) {
    if (node->type == RegexNodeType::EPSILON) {
        size_t s = nfa.add_state();
        size_t a = nfa.add_state();
        nfa.add_epsilon(s, a);
        return {s, a};
    }
    
    if (node->type == RegexNodeType::CHAR) {
        size_t s = nfa.add_state();
        size_t a = nfa.add_state();
        nfa.add_transition(s, node->value, a);
        return {s, a};
    }
    
    if (node->type == RegexNodeType::CONCAT) {
        NFAFragment left = build_fragment(node->op1, nfa);
        NFAFragment right = build_fragment(node->op2, nfa);
        nfa.add_epsilon(left.accept, right.start);
        return {left.start, right.accept};
    }
    
    if (node->type == RegexNodeType::ALTER) {
        size_t s = nfa.add_state();
        size_t a = nfa.add_state();
        NFAFragment left = build_fragment(node->op1, nfa);
        NFAFragment right = build_fragment(node->op2, nfa);
        nfa.add_epsilon(s, left.start);
        nfa.add_epsilon(s, right.start);
        nfa.add_epsilon(left.accept, a);
        nfa.add_epsilon(right.accept, a);
        return {s, a};
    }
    
    // STAR (write it with an if and do an exception after like unsuported regex
    // node type ??)
    size_t s = nfa.add_state();
    size_t a = nfa.add_state();
    NFAFragment inner = build_fragment(node->op1, nfa);
    nfa.add_epsilon(s, inner.start);
    nfa.add_epsilon(s, a);
    nfa.add_epsilon(inner.accept, inner.start);
    nfa.add_epsilon(inner.accept, a);
    return {s, a};
}

void NFA::build(const RegexNode* root) {
    states.clear();
    NFAFragment f = build_fragment(root, *this);
    initial_state = f.start;
    accepting_state = f.accept;
}

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

void DFA::build(const NFA& nfa) {
    // To do (Louis)
}
