#include "qcs.hpp"
#include <cstdio>

void qcs::simulator::alloc_qubit(int n)
{
    fprintf(stderr, "[qubit declare] %d\n", n);
}

void qcs::simulator::gate_matrix(math::matrix_t matrix, int tgt, int const *pc_list, int num_pcs, int const *nc_list, int num_ncs)
{
    fprintf(stderr, "gate matrix={");
#pragma unroll
    for (int i = 0; i < 4; ++i)
    {
        fprintf(stderr, "{%lf, %lf}", matrix[i].real(), matrix[i].imag());
        if (i < 3)
        {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "} tgt=%d pc={", tgt);
    for (int pc_num = 0; pc_num < num_pcs; ++pc_num)
    {
        fprintf(stderr, "%d", pc_list[pc_num]);
        if (pc_num < num_pcs - 1)
        {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "} nc={");
    for (int nc_num = 0; nc_num < num_ncs; ++nc_num)
    {
        fprintf(stderr, "%d", nc_list[nc_num]);
        if (nc_num < num_ncs - 1)
        {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "}\n");
}
