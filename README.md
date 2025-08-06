# qasm-shim

This repository contains a lightweight C++ shim that mimics parts of the
OpenQASM 3 syntax. User circuits derive from `qasm::qasm` and implement
the `circuit()` method. Qubits are allocated with `qalloc`, classical bits
with `clalloc`, and helper methods such as `reset` and `measure` are provided.

Gate expressions are composed using a small builder DSL (e.g. `h()`,
`x()`, `u()`, `cu()`, `ctrl()`, `negctrl()`, `pow()`, `inv()`) and are
forwarded to a `qcs::simulator` backend.

The `qcs` subdirectory provides a minimal stub simulator that logs
operations to `stderr`. Other simulators can integrate with the shim by
supplying a compatible implementation of the `qcs::simulator` interface
defined in `qcs/include/qcs/qcs.hpp`. The location of the simulator
implementation can be overridden with the `QCS` make variable when building.

## Building

```sh
# Build the example circuit and main executable
make all

# Build and run the example
make run
```

`make all` produces `main` and `userqasm.so`. `main` uses `dlopen` to load
`userqasm.so` and execute its `circuit` method. The default circuit is
`src/userqasm_ghz.cpp`; another example `src/userqasm_001.cpp` is provided
for reference.

To link against a different simulator implementation:

```sh
make QCS=/path/to/qcs all
```
