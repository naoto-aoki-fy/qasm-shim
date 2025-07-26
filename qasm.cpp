#include "qasm.hpp"
#include "qcs.hpp"
#include "math.hpp"

namespace qasm {

slice_t slice(int first, int last) {
    return slice_t{first, last};
}

set::set(std::initializer_list<int> lst) : indices(lst) {}

qubits::qubits(qasm &ctx, int n) : ctx_(ctx), base_(ctx.next_id_), size_(n) {
    assert(n > 0);
    ctx.next_id_ += n;
    assert(ctx.simulator_ && "simulator not registered");
    ctx.simulator_->alloc_qubit(n);
}

int qubits::operator[](int i) const {
    assert(0 <= i && i < size_);
    return base_ + i;
}

indices_t qubits::operator[](slice_t sl) const {
    assert(0 <= sl.first && sl.first <= sl.last && sl.last < size_);
    indices_t out;
    for (int i = sl.first; i <= sl.last; ++i) {
        out.values.push_back(base_ + i);
    }
    return out;
}

indices_t qubits::operator[](const set &s) const {
    indices_t out;
    for (int v : s.indices) {
        assert(0 <= v && v < size_);
        out.values.push_back(base_ + v);
    }
    return out;
}

indices_t qubits::operator[](std::initializer_list<int> lst) const {
    indices_t out;
    for (int v : lst) {
        assert(0 <= v && v < size_);
        out.values.push_back(base_ + v);
    }
    return out;
}

builder::builder(const qasm &ctx) : ctx_(ctx) {}

builder::builder(const qasm &ctx, token tk) : ctx_(ctx) {
    seq_.push_back(tk);
}

builder::builder(const builder &rhs) : ctx_(rhs.ctx_), seq_(rhs.seq_) {}

builder &builder::operator=(const builder &rhs) {
    if (this != &rhs) {
        seq_ = rhs.seq_;
    }
    return *this;
}

builder builder::operator*(const builder &rhs) const {
    builder out = *this;
    out.seq_.insert(out.seq_.end(), rhs.seq_.begin(), rhs.seq_.end());
    return out;
}

void builder::operator()(const std::vector<int> &argv) const {
    std::vector<int> pos_ctrls, neg_ctrls;
    math::matrix_t mat{};
    double pow_exp = 1.0;
    bool invert = false;
    std::size_t arg_idx = 0;

    for (const auto &t : seq_) {
        switch (t.kind) {
        case token::POS_CTRL:
            pos_ctrls.push_back(argv[arg_idx++]);
            break;
        case token::NEG_CTRL:
            neg_ctrls.push_back(argv[arg_idx++]);
            break;
        case token::POW:
            pow_exp *= t.val;
            break;
        case token::INV:
            invert = !invert;
            break;
        case token::MATRIX:
            mat = t.mat;
            if (pow_exp != 1.0) {
                mat = math::matrix_pow(mat, pow_exp);
            }
            if (invert) {
                mat = math::matrix_inv(mat);
            }
            dispatch(argv[arg_idx++], mat, pos_ctrls, neg_ctrls);
            pos_ctrls.clear();
            neg_ctrls.clear();
            pow_exp = 1.0;
            invert = false;
            break;
        }
    }
    assert(arg_idx == argv.size());
}

void builder::append_arg(std::vector<int> &out, int v) {
    out.push_back(v);
}

void builder::append_arg(std::vector<int> &out, const indices_t &idx) {
    out.insert(out.end(), idx.values.begin(), idx.values.end());
}

void builder::append_args(std::vector<int> &) {}

void builder::dispatch(int tgt, const math::matrix_t &m, const std::vector<int> &pcs,
                       const std::vector<int> &ncs) const {
    ctx_.dispatch(tgt, m, pcs, ncs);
}

void qasm::register_simulator(qcs::simulator *sim) noexcept {
    simulator_ = sim;
}

builder qasm::h() {
    token tk{token::MATRIX};
    tk.mat = math::generate_matrix_hadamard();
    return builder(*this, tk);
}

builder qasm::u(double th, double ph, double la) {
    token tk{token::MATRIX};
    tk.mat = math::generate_matrix_u(th, ph, la);
    return builder(*this, tk);
}

builder qasm::pow(double exp) {
    token tk{token::POW};
    tk.val = exp;
    return builder(*this, tk);
}

builder qasm::inv() {
    token tk{token::INV};
    return builder(*this, tk);
}

builder qasm::sqrt() {
    return pow(0.5) * inv();
}

builder qasm::ctrl(int N) {
    builder out(*this);
    for (int i = 0; i < N; ++i) {
        token tk{token::POS_CTRL};
        out = out * builder(*this, tk);
    }
    return out;
}

builder qasm::negctrl(int N) {
    builder out(*this);
    for (int i = 0; i < N; ++i) {
        token tk{token::NEG_CTRL};
        out = out * builder(*this, tk);
    }
    return out;
}

void qasm::dispatch(int tgt, const math::matrix_t &m, const std::vector<int> &pcs,
                    const std::vector<int> &ncs) const {
    assert(simulator_ && "simulator not registered");
    simulator_->gate_matrix(m, tgt, pcs.data(), static_cast<int>(pcs.size()),
                            ncs.data(), static_cast<int>(ncs.size()));
}

qubits qasm::qalloc(int n) {
    return qubits(*this, n);
}

void qasm::circuit() {
    throw std::runtime_error("circuit not implemented");
}

} // namespace qasm

