#include "qsim.hpp"
#include <cstdio>
struct simulator {};

void declare_num_qubit(int n){
    fprintf(stderr, "[qubit declare] %d\n", n);
}

void gate_h(simulator*, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs){
    fprintf(stderr, "H tgt=%d pc={", tgt);
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

void gate_u(simulator*, double th, double ph, double la, int tgt, int const* pc_list, int num_pcs, int const* nc_list, int num_ncs){
    fprintf(stderr, "U(%lf, %lf, %lf) tgt=%d pc={", th, ph, la, tgt);
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

int main(){
    simulator sim; g_sim=&sim;

    qubits q1(8), q2(8);

    (ctrl<1>() * h())(q1[0], q1[1]);
    (negctrl<1>() * ctrl<1>() * negctrl<1>() * h())(q1[0], q1[1], q1[2], q1[3]);
    (negctrl<1>() * h())(q1[0], q1[1]);
    (ctrl<1>() * negctrl<1>() * ctrl<1>() * pow(0.5) * u(0,0,1.0))(q1[0], q1[1], q1[2], q1[3]);
}