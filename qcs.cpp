#include "qcs.hpp"
#include <cstdio>
#include <algorithm>

namespace qcs {

struct simulator_core {};

simulator::simulator() : core(nullptr), num_qubits(0) {}

void simulator::setup() {}

void simulator::dispose() {}

int simulator::get_num_procs() { return 1; }

int simulator::get_proc_num() { return 0; }

void simulator::promise_qubits(int n) {
    num_qubits = std::max(num_qubits, n);
    fprintf(stderr, "[promise_qubits] %d\n", n);
}

void simulator::ensure_qubits_allocated() {}

void simulator::reset() {}

void simulator::set_zero_state() {}

void simulator::set_sequential_state() {}

void simulator::set_flat_state() {}

void simulator::set_entangled_state() {}

void simulator::set_random_state() {}

void simulator::hadamard(int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    hadamard_pow(1.0, target, std::move(ncs), std::move(pcs));
}

void simulator::hadamard_pow(double exponent, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[hadamard_pow] exp=%lf tgt=%d\n", exponent, target);
}

void simulator::gate_x(double exponent, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    gate_x_pow(exponent, target, std::move(ncs), std::move(pcs));
}

void simulator::gate_x_pow(double exponent, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[gate_x_pow] exp=%lf tgt=%d\n", exponent, target);
}

void simulator::gate_u4(double th, double ph, double la, double ga, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    gate_u4_pow(th, ph, la, ga, 1.0, target, std::move(ncs), std::move(pcs));
}

void simulator::gate_u4_pow(double th, double ph, double la, double ga, double exp, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[gate_u4_pow] th=%lf ph=%lf la=%lf ga=%lf exp=%lf tgt=%d\n", th, ph, la, ga, exp, target);
}

int simulator::measure(int qubit_num) {
    fprintf(stderr, "[measure] %d\n", qubit_num);
    return 0;
}

} // namespace qcs
