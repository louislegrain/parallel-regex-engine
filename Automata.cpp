#include <map>
#include <set>
#include <stack>
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

std::set<size_t> NFA::epsilon_closure(const std::set<size_t>& initial_states) const {
    std::set<size_t> res;
    std::stack<size_t> stack;
    for (size_t state : initial_states) {
        res.insert(state);
        stack.push(state);
    }

    while (!stack.empty()) {
        const size_t current = stack.top();
        stack.pop();

        for (auto [transition, next_state] : states[current].transitions) {
            if (transition != EPSILON || res.contains(next_state)) continue;
            res.insert(next_state);
            stack.push(next_state);
        }
    }

    return res;
}

std::set<size_t> NFA::move(const std::set<size_t>& initial_states, const char transition) const {
    std::set<size_t> res;
    if (transition == EPSILON) return res;

    for (const size_t state : initial_states) {
        for (auto [next_transition, next_state] : states[state].transitions) {
            if (next_transition == transition) res.insert(next_state);
        }
    }

    return res;
}

void DFA::build(const NFA& nfa) {
    initial_state = 0;
    states.clear();

    std::map<std::set<size_t>, size_t> set_idx;
    std::stack<std::set<size_t>> stack;

    const std::set<size_t> initial_closure = nfa.epsilon_closure({nfa.initial_state});
    set_idx[initial_closure] = 0;
    stack.push(initial_closure);

    states.emplace_back();
    states[0].accepting = initial_closure.contains(nfa.accepting_state);

    while (!stack.empty()) {
        std::set<size_t> current_closure = stack.top();
        stack.pop();
        const size_t current_idx = set_idx[current_closure];

        for (int c = 1; c < 256; ++c) { // start at 1 because 0 is epsilon
            std::set<size_t> next_set = nfa.move(current_closure, static_cast<char>(c));
            if (next_set.empty()) continue;

            std::set<size_t> next_closure = nfa.epsilon_closure(next_set);

            auto [it, inserted] = set_idx.try_emplace(next_closure, states.size());
            if (inserted) {
                stack.push(next_closure);
                states.emplace_back();
                states.back().accepting = next_closure.contains(nfa.accepting_state);
            }

            states[current_idx].transitions[c] = it->second;
        }
    }
}
