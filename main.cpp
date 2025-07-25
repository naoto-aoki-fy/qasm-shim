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



QCS_CONSTEXPR26 qcs::matrix_t qcs::matrix_pow(qcs::matrix_t m, double exponent)
{
    using cmplx = std::complex<double>;
    cmplx a = m[0];
    cmplx b = m[1];
    cmplx c = m[2];
    cmplx d = m[3];
    if (std::abs(b) < 1e-12 && std::abs(c) < 1e-12) {
        return qcs::matrix_t{std::pow(a, exponent), cmplx{}, cmplx{}, std::pow(d, exponent)};
    }
    cmplx trace = a + d;
    cmplx det   = a*d - b*c;
    cmplx disc  = std::sqrt(trace*trace - cmplx{4.0,0.0}*det);
    cmplx lam1  = (trace + disc) / cmplx{2.0,0.0};
    cmplx lam2  = (trace - disc) / cmplx{2.0,0.0};

    cmplx v1_0 = b != cmplx{0.0,0.0} ? cmplx{1.0,0.0} : -(lam1 - d)/c;
    cmplx v1_1 = b != cmplx{0.0,0.0} ? (lam1 - a)/b : cmplx{1.0,0.0};
    cmplx v2_0 = b != cmplx{0.0,0.0} ? cmplx{1.0,0.0} : -(lam2 - d)/c;
    cmplx v2_1 = b != cmplx{0.0,0.0} ? (lam2 - a)/b : cmplx{1.0,0.0};

    cmplx P0 = v1_0, P1 = v2_0, P2 = v1_1, P3 = v2_1;
    cmplx detP = P0*P3 - P1*P2;
    cmplx iP0 =  P3/detP;
    cmplx iP1 = -P1/detP;
    cmplx iP2 = -P2/detP;
    cmplx iP3 =  P0/detP;

    cmplx d1 = std::pow(lam1, exponent);
    cmplx d2 = std::pow(lam2, exponent);

    cmplx B0 = d1 * iP0;
    cmplx B1 = d1 * iP1;
    cmplx B2 = d2 * iP2;
    cmplx B3 = d2 * iP3;

    qcs::matrix_t out;
    out[0] = P0*B0 + P1*B2;
    out[1] = P0*B1 + P1*B3;
    out[2] = P2*B0 + P3*B2;
    out[3] = P2*B1 + P3*B3;
    return out;
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