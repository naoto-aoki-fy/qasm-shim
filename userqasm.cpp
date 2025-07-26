#include "qasm.hpp"

class userqasm : public qasm::qasm
{
public:
    void circuit() {
        using namespace qasm;

        qubits q1 = qalloc(8);
        qubits q2 = qalloc(8);
        (negctrl<2>() * ctrl<2>() * h())(q1[0], q1[slice(1, 2)], q1[{3, 4}]);
        u(0, 0, 1.0)(q1[0]);
        u(0, 0, 0.5)(q1[0]);
        (ctrl<2>() * pow(0.5) * u(0, 0, 1.0))(q1[0], q1[1], q1[2]);
        (inv() * h())(q1[0]);
        (ctrl<2>() * h())(q2[0], q2[1], q2[2]);

    }
};

extern "C" qasm::qasm* constructor() { return new userqasm(); }
