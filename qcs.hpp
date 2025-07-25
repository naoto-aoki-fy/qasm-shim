#pragma once

#include <cmath>
#include <array>
#include <complex>

namespace qcs {
    struct simulator {};

    typedef std::complex<double> std_complex_t;
    typedef std::array<std_complex_t, 4> matrix_t;

    /*-------------------------------------------------------
     * constexpr utility operations for std_complex_t
     *------------------------------------------------------*/
    constexpr std_complex_t cadd(std_complex_t x, std_complex_t y) {
        return std_complex_t{x.real() + y.real(), x.imag() + y.imag()};
    }

    constexpr std_complex_t csub(std_complex_t x, std_complex_t y) {
        return std_complex_t{x.real() - y.real(), x.imag() - y.imag()};
    }

    constexpr std_complex_t cmul(std_complex_t x, std_complex_t y) {
        return std_complex_t{x.real()*y.real() - x.imag()*y.imag(),
                             x.real()*y.imag() + x.imag()*y.real()};
    }

    constexpr std_complex_t cdiv(std_complex_t x, std_complex_t y) {
        double den = y.real()*y.real() + y.imag()*y.imag();
        return std_complex_t{(x.real()*y.real() + x.imag()*y.imag())/den,
                             (x.imag()*y.real() - x.real()*y.imag())/den};
    }
    

    void alloc_qubit(int n);
    
    void gate_matrix(simulator*, matrix_t matrix, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs);
    
    matrix_t matrix_hadamard = { M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, - M_SQRT1_2 };

    constexpr matrix_t generate_matrix_u(double theta, double phi, double lambda);

#if __cplusplus > 202302L
#  define QCS_CONSTEXPR26 constexpr
#else
#  define QCS_CONSTEXPR26
#endif

    QCS_CONSTEXPR26 matrix_t matrix_pow(matrix_t matrix_in, double exponent);

    constexpr matrix_t matrix_inv(matrix_t matrix_in) {
        std_complex_t det = csub(cmul(std::get<0>(matrix_in), std::get<3>(matrix_in)),
                                cmul(std::get<1>(matrix_in), std::get<2>(matrix_in)));
        return matrix_t{
            cdiv(std::get<3>(matrix_in), det),
            cdiv(std_complex_t{-std::get<1>(matrix_in).real(), -std::get<1>(matrix_in).imag()}, det),
            cdiv(std_complex_t{-std::get<2>(matrix_in).real(), -std::get<2>(matrix_in).imag()}, det),
            cdiv(std::get<0>(matrix_in), det)
        };
    }
    
    }