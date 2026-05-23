#include <algorithm>
#include <set>
#include "PaREMMatcher.h"

// Source = PaREM: A Novel Approach for Parallel Regular Expression Matching

PaREMMatcher::PaREMMatcher(const DFA& dfa) : dfa(dfa) {}

// Algo 1, l3
std::vector<PaREMMatcher::Chunk> PaREMMatcher::split_text(size_t n, size_t p) const {
    std::vector<Chunk> chunks;
    for (size_t i = 0; i < p; i++) {
        size_t begin = n * i / p;
        size_t end = n * (i + 1) / p;
        chunks.push_back({begin, end});
    }
    return chunks;
}

// Algo 1, l5-15
// S = states with a valid transition on first_char (l6)
// L = destination states reachable via prev_last_char (l12)
std::vector<size_t> PaREMMatcher::possible_starts(char first_char, char prev_last_char) const {
    std::set<size_t> S, L;

    for (size_t q = 0; q < dfa.size(); q++) {
        if (dfa.step(q, first_char) != INVALID_STATE)
            S.insert(q);

        size_t dest = dfa.step(q, prev_last_char);
        if (dest != INVALID_STATE)
            L.insert(dest);
    }

    std::vector<size_t> R;
    for (size_t q = 0; q < dfa.size(); q++) {
        if (S.count(q) && L.count(q))
            R.push_back(q);
    }

    return R;
}

PaREMResult PaREMMatcher::match(const std::string& text, size_t p) const {
    if (text.empty()) {
        return {dfa.is_accepting(dfa.initial_state), 0};
    }
    if (p == 0) {
        p = 1;
    }
    if (p > text.size()) {
        p = text.size();
    }

    std::vector<Chunk> chunks = split_text(text.size(), p);

    // TODO parallel phase
    // TODO reduction
    return {};
}

