#ifndef PARALLEL_REGEX_ENGINE_AUTOMATA_H
#define PARALLEL_REGEX_ENGINE_AUTOMATA_H
#include <array>
#include <limits>
#include <vector>
#include "RegexParser.h"

constexpr size_t INVALID_STATE = std::numeric_limits<size_t>::max();

struct NFAState {
    std::vector<std::pair<char, size_t>> transitions;
};

class NFA {
private:
    size_t initial_state = 0;
    size_t accepting_state = 0;
    std::vector<NFAState> states;
public:
    void build(const RegexNode* root);
};

struct DFAState {
    std::array<size_t, 256> transitions{}; // O(1) lookup
    bool accepting = false;

    DFAState() { transitions.fill(INVALID_STATE); }
};

class DFA {
private:
    size_t initial_state = 0;
    std::vector<DFAState> states;
public:
    void build(const NFA& nfa);
};

#endif //PARALLEL_REGEX_ENGINE_AUTOMATA_H