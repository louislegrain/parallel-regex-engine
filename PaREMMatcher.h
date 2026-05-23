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

    const DFA& dfa;

    std::vector<Chunk> split_text(size_t n, size_t p) const;
    std::vector<size_t> possible_starts(char first_char, char prev_last_char) const;
};

#endif //PARALLEL_REGEX_ENGINE_PAREMMATCHER_H