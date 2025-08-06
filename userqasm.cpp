#include "qasm.hpp"

class userqasm : public qasm::qasm
{
public:
    void circuit() {
        using namespace qasm;

        qubits q = qalloc(2);
        clbits c = clalloc(2);

        cu(1.0, 0.0, 0.0, 1.0)(q[0]);
        reset(q);
        measure(q);
    }
};

extern "C" qasm::qasm* constructor() { return new userqasm(); }

