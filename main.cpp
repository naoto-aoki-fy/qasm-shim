#include "qcs.hpp"
#include "qasm.hpp"
#include <cstdio>


void qcs::alloc_qubit(int n){
    fprintf(stderr, "[qubit declare] %d\n", n);
}

void qcs::gate_matrix(simulator*, qcs::matrix_t matrix, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs){
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



constexpr qcs::matrix_t qcs::generate_matrix_u(double theta, double phi, double lambda) {
    return qcs::matrix_t{
        qcs::std_complex_t{0.5 * (1 + cos(theta)), 0.5 * sin(theta)},
        qcs::std_complex_t{0.5 * (sin(lambda) - sin(lambda + theta)), -0.5 * (cos(lambda) - cos(lambda + theta))},
        qcs::std_complex_t{- 0.5 * (sin(phi) - sin(phi + theta)), 0.5 * (cos(phi) - cos(phi + theta))},
        qcs::std_complex_t{0.5 * (cos(phi + lambda) + cos(phi + lambda + theta)), 0.5 * (sin(phi + lambda) + sin(phi + lambda + theta))}
    };
};


qcs::matrix_t matrix_inv(qcs::matrix_t m)
{
    return qcs::matrix_t();
}

qcs::matrix_t matrix_pow(qcs::matrix_t m, double exponent)
{
    return qcs::matrix_t();
}

int main() {

    qcs::simulator sim;
    qasm::register_simulator(&sim);

    qasm::qubits q1(8), q2(8);

    (qasm::negctrl<1>() * qasm::ctrl<1>() * qasm::negctrl<1>() * qasm::h())(q1[0], q1[1], q1[2], q1[3]);
    qasm::u(0, 0, 1.0)(q1[0]);
    qasm::u(0, 0, 0.5)(q1[0]);
    (qasm::ctrl<1>() * qasm::negctrl<1>() * qasm::ctrl<1>() * qasm::pow(0.5) * qasm::u(0, 0, 1.0))(q1[0], q1[1], q1[2], q1[3]);
    (qasm::inv() * qasm::h())(q1[0]);
    (qasm::ctrl<1>() * qasm::h())(q2[0], q2[1]);
}