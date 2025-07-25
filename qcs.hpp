#pragma once

#include <cmath>
#include <array>
#include <complex>

namespace qcs {
    struct simulator {};

    typedef std::complex<double> std_complex_t;
    typedef std::array<std_complex_t, 4> matrix_t;
    

    void alloc_qubit(int n);
    
    void gate_matrix(simulator*, matrix_t matrix, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs);
    
    matrix_t matrix_hadamard = { M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, - M_SQRT1_2 };

    constexpr matrix_t generate_matrix_u(double theta, double phi, double lambda);
    matrix_t matrix_pow(matrix_t matrix_in, double exponent);
    
    matrix_t matrix_inv(matrix_t matrix_in);
    
    }