#include <qasm/qasm.hpp>
#include <cstdio>

class userqasm : public qasm::qasm
{
public:
    ::qasm::bit clbit;
    void circuit() {
        using namespace qasm;
        constexpr int num_qubits = 14;
        qubits q = qalloc(num_qubits);
        clbit = clalloc(num_qubits);
        reset(q);
        
        h()(q[0]);
        for (int qubit_num : slice(1, num_qubits - 1)) {
            (ctrl() * x())(q[0], q[qubit_num]);
        }
        clbit = measure(q);
        for (int qubit_num : slice(1, num_qubits - 1)) {
            fprintf(stderr, "%d: %d\n", qubit_num, clbit[qubit_num]);
        }
    }
};

extern "C" qasm::qasm* constructor() { return new userqasm(); }
