#ifndef PARALLEL_REGEX_ENGINE_PAREMMATCHER_H
#define PARALLEL_REGEX_ENGINE_PAREMMATCHER_H
#include <string>
#include <vector>
#include "Automata.h"

// Source = PaREM: A Novel Approach for Parallel Regular Expression Matching

struct PaREMResult {
    bool accepted = false;
    size_t found = 0; // nb of accepting state visits ("found" in paper)
};

class PaREMMatcher {
public:
    PaREMMatcher(const DFA& dfa);
    PaREMResult match(const std::string& text, size_t p) const;
private:
    struct Chunk { // chunk of text
        size_t begin;
        size_t end;
    };

    struct Rr { // notation from Algo 1 (a sort of path / route)
        size_t start_state;
        size_t first_state;
        size_t end_state;
        size_t found;
    };

    const DFA& dfa;

    std::vector<Chunk> split_text(size_t n, size_t p) const;
    std::vector<size_t> possible_starts(char first_char, char prev_last_char) const;
    bool run_from(size_t start_state, const std::string& text, size_t begin, size_t end, Rr& rr) const;
};

#endif //PARALLEL_REGEX_ENGINE_PAREMMATCHER_H