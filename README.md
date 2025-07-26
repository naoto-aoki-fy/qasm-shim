# qasm_shim

This project provides a small C++ shim that mimics parts of OpenQASM 3 style syntax. It allows user-defined quantum circuits written in C++ to call a simulator backend through a simple API.

The shim exposes an interface (`qcs`) for allocating qubits and dispatching gate matrices. Circuits are defined by subclassing `qasm::qasm` and overriding the `circuit()` method. Gates can be combined with a builder pattern to specify control bits, powers, and inverse operations.

A minimal stub simulator is included which prints operations to `stderr`. Other simulators can implement the same API (`alloc_qubit` and `gate_matrix`) to integrate with the shim.

## Building

Run `build.sh` to compile the example circuit (`userqasm.cpp`) into `userqasm.so` and link the main executable:

```sh
make all
```

This will generate `main` and `userqasm.so`. Executing `./main` will load the user circuit and forward operations to the simulator stub.
