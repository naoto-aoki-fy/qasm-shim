#pragma once

#include "math.hpp"

namespace qcs {
    struct simulator {};

    void alloc_qubit(int n);

    void gate_matrix(simulator*, math::matrix_t matrix, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs);
}
