#include "Automata.h"

// Useful for the McNaughton–Yamada–Thompson algorithm
// See also
// https://dmitrysoshnikov.medium.com/building-a-regexp-machine-part-2-finite-automata-nfa-fragments-5a7c5c005ef0
struct NFAFragment {
    size_t start;
    size_t accept;
};

size_t NFA::add_state() {
    states.emplace_back();
    return states.size() - 1;
}

void NFA::add_transition(const size_t from, const size_t to, const char c) {
    states[from].transitions.emplace_back(c, to);
}

NFAFragment build_fragment(const RegexNode* node, NFA& nfa) {
    if (node->type == RegexNodeType::EPSILON) {
        const size_t s = nfa.add_state();
        const size_t a = nfa.add_state();
        nfa.add_transition(s, a, EPSILON);
        return {s, a};
    }
    
    if (node->type == RegexNodeType::CHAR) {
        const size_t s = nfa.add_state();
        const size_t a = nfa.add_state();
        nfa.add_transition(s, a, node->value);
        return {s, a};
    }
    
    if (node->type == RegexNodeType::CONCAT) {
        const NFAFragment left = build_fragment(node->op1, nfa);
        const NFAFragment right = build_fragment(node->op2, nfa);
        nfa.add_transition(left.accept, right.start, EPSILON);
        return {left.start, right.accept};
    }
    
    if (node->type == RegexNodeType::ALTER) {
        const size_t s = nfa.add_state();
        const size_t a = nfa.add_state();
        const NFAFragment left = build_fragment(node->op1, nfa);
        const NFAFragment right = build_fragment(node->op2, nfa);
        nfa.add_transition(s, left.start, EPSILON);
        nfa.add_transition(s, right.start, EPSILON);
        nfa.add_transition(left.accept, a, EPSILON);
        nfa.add_transition(right.accept, a, EPSILON);
        return {s, a};
    }
    
    // STAR (write it with an if and do an exception after like unsuported regex
    // node type ??)
    const size_t s = nfa.add_state();
    const size_t a = nfa.add_state();
    const NFAFragment inner = build_fragment(node->op1, nfa);
    nfa.add_transition(s, inner.start, EPSILON);
    nfa.add_transition(s, a, EPSILON);
    nfa.add_transition(inner.accept, inner.start, EPSILON);
    nfa.add_transition(inner.accept, a, EPSILON);
    return {s, a};
}

void NFA::build(const RegexNode* root) {
    states.clear();
    const NFAFragment f = build_fragment(root, *this);
    initial_state = f.start;
    accepting_state = f.accept;
}

size_t DFA::step(const size_t state, const char c) const {
    if (state >= states.size()) {
        return INVALID_STATE;
    }
    return states[state].transitions[static_cast<unsigned char>(c)];
}

bool DFA::accepts(const std::string& text) const {
    size_t state = initial_state;
    for (const char c : text) {
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
