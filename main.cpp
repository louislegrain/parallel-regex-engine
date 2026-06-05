#include <omp.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include "Automata.h"
#include "PaREMMatcher.h"
#include "RegexParser.h"

double ms_between(const std::chrono::high_resolution_clock::time_point a, const std::chrono::high_resolution_clock::time_point b) {
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count()) / 1e6;
}

DFA build_dfa(const std::string& regex) {
    RegexParser parser;
    const RegexNode* root = parser.parse(regex);
    NFA nfa;
    nfa.build(root);
    DFA dfa;
    dfa.build(nfa);
    delete root;

    return dfa;
}

double time_dfa_build(const std::string& regex, const size_t reps, std::uint64_t& checksum) {
    double total = 0;
    for (size_t i = 0; i < reps; ++i) {
        RegexParser parser;
        const auto t0 = std::chrono::high_resolution_clock::now();
        const RegexNode* root = parser.parse(regex);
        NFA nfa;
        nfa.build(root);
        DFA dfa;
        dfa.build(nfa);
        const auto t1 = std::chrono::high_resolution_clock::now();
        total += ms_between(t0, t1);
        checksum += dfa.size();
        delete root;
    }

    return total / reps;
}

double time_sfa_build(const DFA& dfa, const size_t reps, std::uint64_t& checksum) {
    double total = 0;
    for (size_t i = 0; i < reps; ++i) {
        SFA sfa;
        const auto t0 = std::chrono::high_resolution_clock::now();
        sfa.build(dfa);
        const auto t1 = std::chrono::high_resolution_clock::now();
        total += ms_between(t0, t1);
        checksum += sfa.size();
    }

    return total / reps;
}

std::mt19937 rng(42);

// P1: (a|b)*c -> (n-1) random {a,b} then c
std::string gen_p1(const size_t n) {
    std::string s(n, 'a');
    std::uniform_int_distribution bit(0, 1);

    for (size_t i = 0; i + 1 < n; ++i) {
        s[i] = bit(rng) ? 'b' : 'a';
    }
    s[n - 1] = 'c';

    return s;
}

// P2: abc+d?ef*g -> ab + k*c + optional d + e + m*f + g
std::string gen_p2(const size_t n) {
    // fixed chars: a b c e g
    // remaining split between c (>=1), d (0 or 1) and f (>=0)
    const bool include_d = n >= 6;
    const size_t fixed_len = include_d ? 6 : 5;
    const size_t rem = n > fixed_len ? n - fixed_len : 0;
    const size_t k = 1 + rem / 2;
    const size_t m = rem - rem / 2;

    std::string s;
    s.reserve(n);
    s += "ab";
    s.append(k, 'c');
    if (include_d) s += 'd';
    s += 'e';
    s.append(m, 'f');
    s += 'g';

    return s;
}

double time_seq_dfa(const DFA& dfa, const std::string& text, const size_t reps, std::uint64_t& checksum) {
    checksum += dfa.accepts(text); // warmup

    double best = std::numeric_limits<double>::max();
    for (size_t i = 0; i < reps; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const bool res = dfa.accepts(text);
        const auto t1 = std::chrono::high_resolution_clock::now();
        best = std::min(best, ms_between(t0, t1));
        checksum += res;
    }

    return best;
}

double time_parem(const PaREMMatcher& m, const std::string& text, const size_t p, const size_t reps, std::uint64_t& checksum) {
    checksum += m.match(text, p).accepted; // warmup

    double best = std::numeric_limits<double>::max();
    for (size_t i = 0; i < reps; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        auto [accepted, found] = m.match(text, p);
        const auto t1 = std::chrono::high_resolution_clock::now();
        best = std::min(best, ms_between(t0, t1));
        checksum += accepted;
    }

    return best;
}

double time_sfa_par(const SFA& sfa, const std::string& text, const size_t p, const size_t reps, std::uint64_t& checksum) {
    omp_set_num_threads(p);
    checksum += sfa.accepts_parallel(text); // warmup

    double best = std::numeric_limits<double>::max();
    for (size_t i = 0; i < reps; ++i) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const bool res = sfa.accepts_parallel(text);
        const auto t1 = std::chrono::high_resolution_clock::now();
        best = std::min(best, ms_between(t0, t1));
        checksum += res;
    }

    return best;
}

// average number of candidate start states tried per chunk boundary (chunk 0 starts from q0 so we skip it)
double mean_R(const DFA& dfa, const std::string& text, size_t p) {
    const size_t n = text.size();
    if (p < 2 || n < 2) return 0; // need at least 2 chunks and 2 chars for a boundary
    if (p > n) p = n;

    std::vector<char> can_start(dfa.size());
    std::vector<char> reachable(dfa.size());
    double total = 0;

    for (size_t i = 1; i < p; ++i) {
        const size_t begin = n * i / p;
        const char first_char = text[begin];
        const char prev_char = text[begin - 1];

        std::fill(can_start.begin(), can_start.end(), 0);
        std::fill(reachable.begin(), reachable.end(), 0);
        for (size_t q = 0; q < dfa.size(); ++q) {
            if (dfa.step(q, first_char) != INVALID_STATE) can_start[q] = 1;
            const size_t dest = dfa.step(q, prev_char);
            if (dest != INVALID_STATE) reachable[dest] = 1;
        }
        for (size_t q = 0; q < dfa.size(); ++q) {
            if (can_start[q] && reachable[q]) ++total;
        }
    }

    return total / (p - 1);
}

void check_correctness(const std::string& regex, const std::string& input) {
    const DFA dfa = build_dfa(regex);
    SFA sfa;
    sfa.build(dfa);
    const PaREMMatcher parem(dfa);

    const bool seq = dfa.accepts(input);
    const bool pa = parem.match(input, 4).accepted;
    const bool ss = sfa.accepts_sequential(input);
    const bool sp = sfa.accepts_parallel(input);

    const std::string repr[] = {"reject", "accept"}; // repr[false] -> reject, repr[true] -> accept
    const bool agree = (seq == pa) && (seq == ss) && (seq == sp);
    std::cout << std::left << std::setw(15) << regex << " | "
              << std::setw(10) << input << " | "
              << std::setw(6) << repr[seq] << " | "
              << std::setw(6) << repr[pa] << " | "
              << std::setw(6) << repr[ss] << " | "
              << std::setw(6) << repr[sp] << " | "
              << (agree ? "OK" : "MISMATCH") << std::endl;
}

int main() {
    std::cout << std::fixed << std::setprecision(4); // print numbers with fixed 4 digits

    std::uint64_t checksum = 0; // avoid compiler optimizations that would skip calls we are timing

    const std::string P1 = "(a|b)*c";
    const std::string P2 = "abc+d?ef*g";
    const std::string P3 = "(ab)*";
    const std::string P4 = "((a|b)*c|d+)?e*";

    constexpr size_t REPS = 25;
    const std::vector<size_t> THREADS = {1, 2, 3, 4, 6, 8};
    const std::vector<size_t> SIZES = {10000, 100000, 1000000, 10000000};

    std::cout << "--- CORRECTNESS ---" << std::endl;
    std::cout << std::left << std::setw(15) << "Regex" << " | "
              << std::setw(10) << "Input" << " | "
              << std::setw(6) << "Seq" << " | "
              << std::setw(6) << "PaREM" << " | "
              << std::setw(6) << "SFA(s)" << " | "
              << std::setw(6) << "SFA(p)" << " | Result" << std::endl;
    check_correctness("(ab)*", "ababab");
    check_correctness("(ab)*", "aba");
    check_correctness("(a|b)*c", "aabbc");
    check_correctness("(a|b)*c", "aabb");
    check_correctness("((a|b)*c|d+)?e*", "abce");
    check_correctness("((a|b)*c|d+)?e*", "ddd");

    std::cout << std::endl
              << "--- STATES ---" << std::endl;
    std::cout << std::left << std::setw(7) << "Pattern" << " | "
              << std::setw(15) << "Regex" << " | "
              << std::setw(10) << "DFA States" << " | "
              << std::setw(10) << "SFA States" << " | "
              << std::setw(13) << "DFA Build(ms)" << " | SFA Build(ms)" << std::endl;
    std::vector<std::pair<std::string, std::string>> patterns = {{"P1", P1}, {"P2", P2}, {"P3", P3}, {"P4", P4}};
    for (auto& [name, regex] : patterns) {
        DFA dfa = build_dfa(regex);
        SFA sfa;
        sfa.build(dfa);
        double dfa_build = time_dfa_build(regex, 1000, checksum);
        double sfa_build = time_sfa_build(dfa, 1000, checksum);
        std::cout << std::left << std::setw(7) << name << " | "
                  << std::setw(15) << regex << " | "
                  << std::setw(10) << dfa.size() << " | "
                  << std::setw(10) << sfa.size() << " | "
                  << std::setw(13) << dfa_build << " | "
                  << sfa_build << std::endl;
    }

    std::cout << std::endl
              << "--- GRID ---" << std::endl;
    std::vector<std::pair<std::string, std::string>> benchmarks = {{"P1", P1}, {"P2", P2}};
    for (auto& [name, regex] : benchmarks) {
        DFA dfa = build_dfa(regex);
        SFA sfa;
        sfa.build(dfa);
        PaREMMatcher parem(dfa);

        for (size_t n : SIZES) {
            std::string text = name == "P1" ? gen_p1(n) : gen_p2(n);
            bool agree = dfa.accepts(text) && parem.match(text, 4).accepted && sfa.accepts_sequential(text) && sfa.accepts_parallel(text);
            std::cout << std::endl
                      << "Testing " << name << " (size n=" << n << ") - Sanity check: " << (agree ? "OK" : "FAILED") << std::endl;
            std::cout << std::left << std::setw(7) << "Threads" << " | "
                      << std::setw(8) << "Seq(ms)" << " | "
                      << std::setw(9) << "PaREM(ms)" << " | "
                      << std::setw(8) << "SFA(ms)" << " | "
                      << std::setw(6) << "mean_R" << " | "
                      << std::setw(13) << "PaREM Speedup" << " | SFA Speedup" << std::endl;

            double tseq = time_seq_dfa(dfa, text, REPS, checksum);
            for (size_t p : THREADS) {
                double tpa = time_parem(parem, text, p, REPS, checksum);
                double tsfa = time_sfa_par(sfa, text, p, REPS, checksum);
                double r = mean_R(dfa, text, p);
                std::cout << std::left << std::setw(7) << p << " | "
                          << std::setw(8) << tseq << " | "
                          << std::setw(9) << tpa << " | "
                          << std::setw(8) << tsfa << " | "
                          << std::setw(6) << r << " | "
                          << std::setw(13) << (tseq / tpa) << " | "
                          << (tseq / tsfa) << std::endl;
            }
        }
    }

    std::cout << std::endl
              << "DONE (checksum=" << checksum << ")" << std::endl;
    return 0;
}
