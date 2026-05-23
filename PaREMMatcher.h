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
    const DFA& dfa;
};

#endif //PARALLEL_REGEX_ENGINE_PAREMMATCHER_H