#include <qcs/qcs.hpp>
#include <cstdio>
#include <algorithm>

namespace qcs {

struct simulator_core {};

static void print_ctrls(const std::vector<int>& ncs, const std::vector<int>& pcs) {
    fprintf(stderr, " negctrl=[");
    for (size_t i = 0; i < ncs.size(); ++i) {
        fprintf(stderr, "%s%d", i ? "," : "", ncs[i]);
    }
    fprintf(stderr, "] ctrl=[");
    for (size_t i = 0; i < pcs.size(); ++i) {
        fprintf(stderr, "%s%d", i ? "," : "", pcs[i]);
    }
    fprintf(stderr, "]");
}

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

void simulator::reset(int qubit_num) {
    fprintf(stderr, "[reset] %d\n", qubit_num);
}

void simulator::set_zero_state() {}

void simulator::set_sequential_state() {}

void simulator::set_flat_state() {}

void simulator::set_entangled_state() {}

void simulator::set_random_state() {}

void simulator::hadamard(int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    hadamard_pow(1.0, target, std::move(ncs), std::move(pcs));
}

void simulator::hadamard_pow(double exponent, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[hadamard_pow] exp=%lf tgt=%d", exponent, target);
    print_ctrls(ncs, pcs);
    fprintf(stderr, "\n");
}

void simulator::gate_x(int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    gate_x_pow(1.0, target, std::move(ncs), std::move(pcs));
}

void simulator::gate_x_pow(double exponent, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[gate_x_pow] exp=%lf tgt=%d", exponent, target);
    print_ctrls(ncs, pcs);
    fprintf(stderr, "\n");
}

void simulator::gate_u4(double th, double ph, double la, double ga, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    gate_u4_pow(th, ph, la, ga, 1.0, target, std::move(ncs), std::move(pcs));
}

void simulator::gate_u4_pow(double th, double ph, double la, double ga, double exp, int target, std::vector<int>&& ncs, std::vector<int>&& pcs) {
    fprintf(stderr, "[gate_u4_pow] th=%lf ph=%lf la=%lf ga=%lf exp=%lf tgt=%d", th, ph, la, ga, exp, target);
    print_ctrls(ncs, pcs);
    fprintf(stderr, "\n");
}

int simulator::measure(int qubit_num) {
    fprintf(stderr, "[measure] %d\n", qubit_num);
    return 0;
}

} // namespace qcs
