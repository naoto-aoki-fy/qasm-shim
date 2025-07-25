#include "qcs.hpp"
#include "qasm.hpp"
#include <cstdio>


void qcs::alloc_qubit(int n){
    fprintf(stderr, "[qubit declare] %d\n", n);
}

void qcs::gate_matrix(simulator*, math::matrix_t matrix, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs){
    fprintf(stderr, "gate matrix={");
    #pragma unroll
    for(int i = 0; i < 4; ++i) {
        fprintf(stderr, "{%lf, %lf}", matrix[i].real(), matrix[i].imag());
        if (i<3) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "} tgt=%d pc={", tgt);
    for(int pc_num = 0; pc_num < num_pcs; ++pc_num) {
        fprintf(stderr, "%d", pc_list[pc_num]);
        if (pc_num<num_pcs-1) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "} nc={");
    for(int nc_num = 0; nc_num < num_ncs; ++nc_num) {
        fprintf(stderr, "%d", nc_list[nc_num]);
        if (nc_num<num_ncs-1) {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "}\n");
}




int main() {

    qcs::simulator sim;
    qasm::qasm q;
    q.register_simulator(&sim);

    qasm::qasm::qubits q1(q, 8), q2(q, 8);

    (q.negctrl<2>() * q.ctrl<2>() * q.h())(q1[0], q1[1], q1[2], q1[3], q1[4]);
    q.u(0, 0, 1.0)(q1[0]);
    q.u(0, 0, 0.5)(q1[0]);
    (q.ctrl<2>() * q.pow(0.5) * q.u(0, 0, 1.0))(q1[0], q1[1], q1[2]);
    (q.inv() * q.h())(q1[0]);
    (q.ctrl<2>() * q.h())(q2[0], q2[1], q2[2]);
}