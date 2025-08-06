#include <cstdio>
#include <dlfcn.h>
#include <stdexcept>
#include <qasm/qasm.hpp>
#include <qcs/qcs.hpp>

int main()
{
    qcs::simulator sim;

    const auto userqasm_dl = dlopen("./userqasm.so", RTLD_LAZY);
    if (userqasm_dl == NULL) { throw std::runtime_error("dlopen failed"); }

    auto userqasm_constructor = reinterpret_cast<qasm::qasm*(*)()>(dlsym(userqasm_dl, "constructor"));

    qasm::qasm* q = userqasm_constructor();
    q->register_simulator(&sim);
    q->circuit();
    delete q;

    int const ret_dlclose = dlclose(userqasm_dl);
    if (ret_dlclose != 0) { throw std::runtime_error("dlclose failed"); }

    return 0;
}