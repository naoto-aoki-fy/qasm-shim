#include <qasm/qasm.hpp>

class userqasm : public qasm::qasm
{
public:
    ::qasm::bit clbit;
    void circuit() {
        using namespace qasm;

        qubits q1 = qalloc(8);
        qubits q2 = qalloc(8);
        clbit = clalloc(16);
        reset(q1);
        reset(q2);
        (negctrl(2) * ctrl(2) * h())(q1[0], q1[slice(1, 2)], q1[{3, 4}]);
        cu(0, 0, 1.0, 0)(q1[0], q1[1]);
        cu(0, 0, 0.5, 0)(q1[0], q1[1]);
        (ctrl(2) * pow(0.5) * cu(0, 0, 1.0, 0))(q1[0], q1[1], q1[2], q1[3]);
        (inv() * h())(q1[0]);
        (ctrl(2) * h())(q2[0], q2[1], q2[2]);

        clbit[slice(0, 7)] = measure(q1[slice(0, 7)]);
        clbit[{8, 9}] = measure(q2[{0, 1}]);

    }
};

extern "C" qasm::qasm* constructor() { return new userqasm(); }
