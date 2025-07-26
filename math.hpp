#pragma once

#include "math_type.hpp"
#include <cmath>

namespace math
{

    inline complex_t cadd(complex_t x, complex_t y)
    {
        return complex_t{x.real() + y.real(), x.imag() + y.imag()};
    }

    inline complex_t csub(complex_t x, complex_t y)
    {
        return complex_t{x.real() - y.real(), x.imag() - y.imag()};
    }

    inline complex_t cmul(complex_t x, complex_t y)
    {
        return complex_t{x.real() * y.real() - x.imag() * y.imag(),
                         x.real() * y.imag() + x.imag() * y.real()};
    }

    inline complex_t cdiv(complex_t x, complex_t y)
    {
        double den = y.real() * y.real() + y.imag() * y.imag();
        return complex_t{(x.real() * y.real() + x.imag() * y.imag()) / den,
                         (x.imag() * y.real() - x.real() * y.imag()) / den};
    }

    inline matrix_t generate_matrix_hadamard() {
        return {M_SQRT1_2, M_SQRT1_2, M_SQRT1_2, -M_SQRT1_2};
    }

    inline matrix_t generate_matrix_u(double theta, double phi, double lambda)
    {
        return matrix_t{
            complex_t{0.5 * (1 + std::cos(theta)), 0.5 * std::sin(theta)},
            complex_t{0.5 * (std::sin(lambda) - std::sin(lambda + theta)), -0.5 * (std::cos(lambda) - std::cos(lambda + theta))},
            complex_t{-0.5 * (std::sin(phi) - std::sin(phi + theta)), 0.5 * (std::cos(phi) - std::cos(phi + theta))},
            complex_t{0.5 * (std::cos(phi + lambda) + std::cos(phi + lambda + theta)), 0.5 * (std::sin(phi + lambda) + std::sin(phi + lambda + theta))}};
    }

    inline matrix_t matrix_pow(matrix_t m, double exponent)
    {
        using cmplx = complex_t;
        cmplx a = m[0];
        cmplx b = m[1];
        cmplx c = m[2];
        cmplx d = m[3];
        if (std::abs(b) < 1e-12 && std::abs(c) < 1e-12)
        {
            return matrix_t{std::pow(a, exponent), cmplx{}, cmplx{}, std::pow(d, exponent)};
        }
        cmplx trace = a + d;
        cmplx det = a * d - b * c;
        cmplx disc = std::sqrt(trace * trace - cmplx{4.0, 0.0} * det);
        cmplx lam1 = (trace + disc) / cmplx{2.0, 0.0};
        cmplx lam2 = (trace - disc) / cmplx{2.0, 0.0};

        cmplx v1_0 = b != cmplx{0.0, 0.0} ? cmplx{1.0, 0.0} : -(lam1 - d) / c;
        cmplx v1_1 = b != cmplx{0.0, 0.0} ? (lam1 - a) / b : cmplx{1.0, 0.0};
        cmplx v2_0 = b != cmplx{0.0, 0.0} ? cmplx{1.0, 0.0} : -(lam2 - d) / c;
        cmplx v2_1 = b != cmplx{0.0, 0.0} ? (lam2 - a) / b : cmplx{1.0, 0.0};

        cmplx P0 = v1_0, P1 = v2_0, P2 = v1_1, P3 = v2_1;
        cmplx detP = P0 * P3 - P1 * P2;
        cmplx iP0 = P3 / detP;
        cmplx iP1 = -P1 / detP;
        cmplx iP2 = -P2 / detP;
        cmplx iP3 = P0 / detP;

        cmplx d1 = std::pow(lam1, exponent);
        cmplx d2 = std::pow(lam2, exponent);

        cmplx B0 = d1 * iP0;
        cmplx B1 = d1 * iP1;
        cmplx B2 = d2 * iP2;
        cmplx B3 = d2 * iP3;

        matrix_t out;
        out[0] = P0 * B0 + P1 * B2;
        out[1] = P0 * B1 + P1 * B3;
        out[2] = P2 * B0 + P3 * B2;
        out[3] = P2 * B1 + P3 * B3;
        return out;
    }

    inline matrix_t matrix_inv(matrix_t matrix_in)
    {
        complex_t det = csub(cmul(std::get<0>(matrix_in), std::get<3>(matrix_in)), cmul(std::get<1>(matrix_in), std::get<2>(matrix_in)));
        return matrix_t{
            cdiv(std::get<3>(matrix_in), det),
            cdiv(complex_t{-std::get<1>(matrix_in).real(), -std::get<1>(matrix_in).imag()}, det),
            cdiv(complex_t{-std::get<2>(matrix_in).real(), -std::get<2>(matrix_in).imag()}, det),
            cdiv(std::get<0>(matrix_in), det)};
    }

} // namespace math
