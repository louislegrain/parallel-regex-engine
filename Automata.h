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
    std::array<size_t, 256> transitions{}; // O(1) lookup, 2^8 one slot for each possible char value
    bool accepting = false;

    DFAState() { transitions.fill(INVALID_STATE); }
};

class DFA {
private:
    size_t initial_state = 0;
    std::vector<DFAState> states;
public:
    void build(const NFA& nfa);
    // use of const because read only methods
    int step(int state, char c) const;           // perform DFA transition
    bool accepts(const std::string &text) const; // sequ DFA matcher
    int size() const;                            // nb of DFA states
};

#endif //PARALLEL_REGEX_ENGINE_AUTOMATA_H