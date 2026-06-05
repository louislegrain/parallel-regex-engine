# Parallel regex engine

A parallel regular expression matching engine that implements and benchmarks the PaREM and Simultaneous Finite Automata (SFA) approaches for high-performance text processing on multicore CPUs.

## Run locally

Build the project

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Then run the executable

```bash
./build/parallel_regex_engine
```

## References

- [S. Memeti and S. Pllana, "PaREM: A Novel Approach for Parallel Regular Expression Matching," 2014 IEEE 17th International Conference on Computational Science and Engineering, Chengdu, China, 2014, pp. 690-697, doi: 10.1109/CSE.2014.146](https://arxiv.org/abs/1412.1741)
- [R. Sinya, K. Matsuzaki and M. Sassa, "Simultaneous Finite Automata: An Efficient Data-Parallel Model for Regular Expression Matching," 2013 42nd International Conference on Parallel Processing, Lyon, France, 2013, pp. 220-229, doi: 10.1109/ICPP.2013.31](https://arxiv.org/abs/1405.0562)
