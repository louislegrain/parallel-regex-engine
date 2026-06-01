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
        if (dfa.step(q, first_char) != INVALID_STATE) {
            S.insert(q);
        }
        size_t dest = dfa.step(q, prev_last_char);
        if (dest != INVALID_STATE) {
            L.insert(dest);
        }
    }

    std::vector<size_t> R;
    for (size_t q = 0; q < dfa.size(); q++) {
        if (S.count(q) && L.count(q)) {
            R.push_back(q);
        }
    }

    return R;
}

// Algo 1, l16-25
bool PaREMMatcher::run_from(size_t start_state, const std::string& text, size_t begin, size_t end, Rr& rr) const {
    size_t state = start_state;
    size_t first_state = INVALID_STATE;
    size_t found = 0;

    for (size_t i = begin; i < end; i++) {
        state = dfa.step(state, text[i]);
        if (state == INVALID_STATE) {
            return false;
        }
        if (i == begin) {
            first_state = state;
        }
        if (dfa.is_accepting(state)) {
            found++;
        }
    }

    rr = {start_state, first_state, state, found};
    return true;
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
    std::vector<std::vector<Rr>> routes(p);

#pragma omp parallel for num_threads(p)
    for (size_t i = 0; i < p; i++) {
        size_t begin = chunks[i].begin;
        size_t end = chunks[i].end;

        // th 0 starts from q0 (Algo 1, ID=0 already knows init state)
        if (i == 0) {
            Rr rr{};
            if (run_from(dfa.initial_state, text, begin, end, rr)) {
                routes[0].push_back(rr);
            }
            continue;
        }

        char first_char = text[begin];
        char prev_last_char = text[chunks[i - 1].end - 1];

        std::vector<size_t> starts = possible_starts(first_char, prev_last_char);
        for (size_t r : starts) {
            Rr rr{};
            if (run_from(r, text, begin, end, rr)) {
                routes[i].push_back(rr);
            }
        }
    }

    if (routes[0].empty()) {
        return {};
    }

    // routes[0] = always exactly 1 entry (bc th 0 starts from q0)
    std::vector<PartialResult> current;
    current.push_back({routes[0][0].end_state, routes[0][0].found});

    for (size_t i = 1; i < p; i++) {
        std::vector<PartialResult> next;
        char fc = text[chunks[i].begin];

        for (size_t j = 0; j < current.size(); j++) {
            size_t expected = dfa.step(current[j].end_state, fc);
            if (expected == INVALID_STATE) {
                continue;
            }
            for (size_t k = 0; k < routes[i].size(); k++) {
                if (routes[i][k].first_state == expected) {
                    next.push_back(
                        {routes[i][k].end_state, current[j].found + routes[i][k].found});
                }
            }
        }

        current = next;
        if (current.empty()) {
            return {};
        }
    }

    for (size_t j = 0; j < current.size(); j++) {
        if (dfa.is_accepting(current[j].end_state)) {
            return {true, current[j].found};
        }
    }

    return {false, current[0].found};
}

