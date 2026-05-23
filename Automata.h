#ifndef PARALLEL_REGEX_ENGINE_AUTOMATA_H
#define PARALLEL_REGEX_ENGINE_AUTOMATA_H
#include <array>
#include <limits>
#include <set>
#include <vector>
#include "RegexParser.h"

constexpr size_t INVALID_STATE = std::numeric_limits<size_t>::max();
constexpr char EPSILON = '\0';

struct NFAState {
    std::vector<std::pair<char, size_t>> transitions;
};

class NFA {
    friend class DFA;

private:
    size_t initial_state = 0;
    size_t accepting_state = 0;
    std::vector<NFAState> states;
public:
    void build(const RegexNode* root);
    size_t add_state();
    void add_transition(size_t from, size_t to, char c);
    [[nodiscard]] std::set<size_t> epsilon_closure(const std::set<size_t>& initial_states) const;
    [[nodiscard]] std::set<size_t> move(const std::set<size_t>& initial_states, char transition) const;
};

struct DFAState {
    std::array<size_t, 256> transitions{}; // O(1) lookup, 2^8 one slot for each possible char value
    bool accepting = false;

    DFAState() { transitions.fill(INVALID_STATE); }
};

class DFA {
    friend class SFA;
    friend class PaREMMatcher;

private:
    size_t initial_state = 0;
    std::vector<DFAState> states;
public:
    void build(const NFA& nfa);
    // use of const because read only methods
    [[nodiscard]] size_t step(size_t state, char c) const;     // perform DFA transition
    [[nodiscard]] bool accepts(const std::string& text) const; // sequ DFA matcher
    [[nodiscard]] size_t size() const;                         // nb of DFA states
    [[nodiscard]] bool is_accepting(size_t state) const;
};

struct SFAState {
    std::vector<size_t> mapping;
    std::array<size_t, 256> transitions{};

    SFAState() { transitions.fill(INVALID_STATE); }
};

class SFA {
private:
    size_t initial_state = 0;
    size_t initial_dfa_state = 0;
    std::vector<SFAState> states;
    std::vector<bool> accepting_dfa_states;
public:
    void build(const DFA& dfa);
    [[nodiscard]] size_t step(size_t state, char c) const;
    [[nodiscard]] bool accepts_sequential(const std::string& text) const;
    [[nodiscard]] bool accepts_parallel(const std::string& text) const;
    [[nodiscard]] size_t size() const;
};

#endif //PARALLEL_REGEX_ENGINE_AUTOMATA_H