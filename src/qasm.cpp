#include <qasm/qasm.hpp>
#include <qcs/qcs.hpp>
#include <utility>
#include <stdexcept>

namespace qasm {

slice_t slice(int first, int last) {
    return slice_t{first, last};
}

set::set(std::initializer_list<int> lst) : indices(lst) {}

qubits::qubits(qasm &ctx, int n) : ctx_(ctx), base_(ctx.next_id_), size_(n) {
    assert(n > 0);
    ctx.next_id_ += n;
    assert(ctx.simulator_ && "simulator not registered");
    ctx.simulator_->promise_qubits(n);
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
        case token::X: {
            double exp = pow_exp * (invert ? -1.0 : 1.0);
            assert(ctx_.simulator_ && "simulator not registered");
            if (exp == 1.0) {
                ctx_.simulator_->gate_x(argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            } else {
                ctx_.simulator_->gate_x_pow(exp, argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            }
            pos_ctrls.clear();
            neg_ctrls.clear();
            pow_exp = 1.0;
            invert = false;
            break;
        }
        case token::HADAMARD: {
            double exp = pow_exp * (invert ? -1.0 : 1.0);
            assert(ctx_.simulator_ && "simulator not registered");
            if (exp==1.0) {
                ctx_.simulator_->hadamard(argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            } else {
                ctx_.simulator_->hadamard_pow(exp, argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            }
            pos_ctrls.clear();
            neg_ctrls.clear();
            pow_exp = 1.0;
            invert = false;
            break;
        }
        case token::U4: {
            double exp = pow_exp * (invert ? -1.0 : 1.0);
            assert(ctx_.simulator_ && "simulator not registered");
            if (exp==1.0) {
                ctx_.simulator_->gate_u4(t.theta, t.phi, t.lambda, t.gamma, argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            } else {
                ctx_.simulator_->gate_u4_pow(t.theta, t.phi, t.lambda, t.gamma, exp, argv[arg_idx++], std::move(neg_ctrls), std::move(pos_ctrls));
            }
            pos_ctrls.clear();
            neg_ctrls.clear();
            pow_exp = 1.0;
            invert = false;
            break;
        }
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

void qasm::register_simulator(qcs::simulator *sim) noexcept {
    simulator_ = sim;
}

builder qasm::h() {
    token tk{token::HADAMARD};
    return builder(*this, tk);
}

builder qasm::x() {
    token tk{token::X};
    return builder(*this, tk);
}

builder qasm::u(double th, double ph, double la) {
    token tk{token::U4};
    tk.theta = th;
    tk.phi = ph;
    tk.lambda = la;
    tk.gamma = 0;
    return builder(*this, tk);
}

builder qasm::cu(double th, double ph, double la, double ga) {
    token ctrl{token::POS_CTRL};
    token u4{token::U4};
    u4.theta = th;
    u4.phi = ph;
    u4.lambda = la;
    u4.gamma = ga;
    return builder(*this, ctrl) * builder(*this, u4);
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

qubits qasm::qalloc(int n) {
    return qubits(*this, n);
}

bit qasm::clalloc(int n) {
    return bit(n);
}

void qasm::reset(const qubits &qs) {
    assert(simulator_ && "simulator not registered");
    for (int i = 0; i < qs.size_; ++i) {
        simulator_->reset(qs.base_ + i);
    }
}

void qasm::reset(const indices_t &qs) {
    assert(simulator_ && "simulator not registered");
    for (int q : qs.values) {
        simulator_->reset(q);
    }
}

std::vector<int> qasm::measure(const qubits &qs) {
    indices_t idx;
    for (int i = 0; i < qs.size_; ++i) {
        idx.values.push_back(qs.base_ + i);
    }
    return measure(idx);
}

std::vector<int> qasm::measure(const indices_t &qs) {
    assert(simulator_ && "simulator not registered");
    std::vector<int> out;
    out.reserve(qs.values.size());
    for (int q : qs.values) {
        out.push_back(simulator_->measure(q));
    }
    return out;
}

void qasm::circuit() {
    throw std::runtime_error("circuit not implemented");
}

} // namespace qasm

