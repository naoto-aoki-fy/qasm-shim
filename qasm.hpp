#pragma once
#include <vector>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>

#include "qcs.hpp"
#include "math.hpp"

namespace qasm {

struct slice_t {
    int first;
    int last;
};

inline slice_t slice(int first, int last) {
    return slice_t{first, last};
}

struct set {
    std::vector<int> indices;
    set(std::initializer_list<int> lst) : indices(lst) {}
};

struct indices_t {
    std::vector<int> values;
};

class qasm {
public:
    qasm() = default;

    /*-------------------------------------------------------
     * 0.  外部 Simulator 登録
     *------------------------------------------------------*/
    void register_simulator(qcs::simulator* sim) noexcept {
        simulator_ = sim;
    }

    /*-------------------------------------------------------
     * 1.  量子レジスタ（連番 ID を払い出すだけ）
     *------------------------------------------------------*/
    class qubits {
    public:
        qubits(qasm& ctx, int n) : ctx_(ctx), base_(ctx.next_id_), size_(n) {
            assert(n > 0);
            ctx.next_id_ += n;
            qcs::alloc_qubit(n);    // 低レイヤへ通知
        }
        int operator[](int i) const {
            assert(0 <= i && i < size_);
            return base_ + i;
        }
        indices_t operator[](slice_t sl) const {
            assert(0 <= sl.first && sl.first <= sl.last && sl.last < size_);
            indices_t out;
            for (int i = sl.first; i <= sl.last; ++i) {
                out.values.push_back(base_ + i);
            }
            return out;
        }
        indices_t operator[](const set& s) const {
            indices_t out;
            for (int v : s.indices) {
                assert(0 <= v && v < size_);
                out.values.push_back(base_ + v);
            }
            return out;
        }
    private:
        qasm& ctx_;
        int base_;
        int size_;
    };

/*-------------------------------------------------------
 * 2.  ビルダーに詰めるトークン
 *------------------------------------------------------*/
struct token {
    enum kind_t { POS_CTRL, NEG_CTRL, MATRIX, POW, INV } kind;
    math::matrix_t mat {};   // MATRIX のときに使用
    double        val = 1;  // POW のときに使用
    explicit token(kind_t k) : kind(k), mat(), val(1) {}
};

/*-------------------------------------------------------
 * 3.  ゲート式ビルダー
 *------------------------------------------------------*/
class builder {
public:
    builder(const qasm& ctx) : ctx_(ctx) {}
    builder(const qasm& ctx, token tk) : ctx_(ctx) { seq_.push_back(tk); }
    builder(const builder& rhs) : ctx_(rhs.ctx_), seq_(rhs.seq_) {}
    builder& operator=(const builder& rhs) {
        if (this != &rhs) {
            seq_ = rhs.seq_;
        }
        return *this;
    }

    /*--- 演算子* でトークン列を連結 ----------------------*/
    builder  operator*(const builder& rhs) const {
        builder out = *this;
        out.seq_.insert(out.seq_.end(), rhs.seq_.begin(), rhs.seq_.end());
        return out;
    }

    /*--- () で量子ビットを与えて実行 --------------------*/
    template<typename... Q>
    void operator()(Q... qs) const {
        static_assert(sizeof...(qs) > 0, "at least one qubit is required");
        std::vector<int> argv;
        argv.reserve(sizeof...(qs));
        append_args(argv, qs...);
        std::vector<int> pos_ctrls, neg_ctrls;
        math::matrix_t     mat {};
        bool              has_mat = false;
        double            pow_exp = 1.0;
        bool              invert  = false;
        std::size_t       arg_idx = 0;

        for (const auto& t : seq_) {
            switch (t.kind) {
            case token::POS_CTRL:
                pos_ctrls.push_back(argv[arg_idx++]); break;
            case token::NEG_CTRL:
                neg_ctrls.push_back(argv[arg_idx++]); break;
            case token::POW:
                pow_exp *= t.val; break;
            case token::INV:
                invert = !invert; break;
            case token::MATRIX:
                mat = t.mat;     // 基本行列
                // apply optional operations
                if (pow_exp != 1.0) {
                    mat = math::matrix_pow(mat, pow_exp);
                }
                if (invert) {
                    mat = math::matrix_inv(mat);
                }
                has_mat = true;
                // 対応する量子ビットは「ターゲット」
                dispatch(argv[arg_idx++], mat,
                         pos_ctrls, neg_ctrls);
                // 状態リセット
                pos_ctrls.clear();
                neg_ctrls.clear();
                pow_exp = 1.0;
                invert  = false;
                has_mat = false;
                break;
            }
        }
        assert(arg_idx == argv.size());
    }

    /*--- 内部用：トークン生成 ----------------------------*/
    explicit builder(token tk) = delete;

private:
    const qasm& ctx_;
    std::vector<token> seq_;

    static void append_arg(std::vector<int>& out, int v) {
        out.push_back(v);
    }
    static void append_arg(std::vector<int>& out, const indices_t& idx) {
        out.insert(out.end(), idx.values.begin(), idx.values.end());
    }
    static void append_args(std::vector<int>&) {}
    template<typename First, typename... Rest>
    static void append_args(std::vector<int>& out, First&& first, Rest&&... rest) {
        append_arg(out, std::forward<First>(first));
        append_args(out, std::forward<Rest>(rest)...);
    }

    void dispatch(int tgt,
                  const math::matrix_t& m,
                  const std::vector<int>& pcs,
                  const std::vector<int>& ncs) const
    {
        ctx_.dispatch(tgt, m, pcs, ncs);
    }
};

/*-------------------------------------------------------
 * 4.  単一量子ゲート生成関数
 *------------------------------------------------------*/
inline builder h() {
    token tk{token::MATRIX};
    tk.mat = math::matrix_hadamard;
    return builder(*this, tk);
}

inline builder u(double th, double ph, double la) {
    token tk{token::MATRIX};
    tk.mat = math::generate_matrix_u(th, ph, la);
    return builder(*this, tk);
}

inline builder pow(double exp) {
    token tk{token::POW};
    tk.val = exp;
    return builder(*this, tk);
}

inline builder inv() {
    token tk{token::INV};
    return builder(*this, tk);
}

inline builder sqrt() {
    return pow(0.5) * inv();
}

/*-------------------------------------------------------
 * 5.  条件付き演算子（N ビット版）
 *------------------------------------------------------*/
template<int N> inline builder ctrl() {
    static_assert(N >= 1, "ctrl<N> requires N >= 1");
    builder out(*this);
    for (int i = 0; i < N; ++i) {
        token tk{token::POS_CTRL};
        out = out * builder(*this, tk);
    }
    return out;
}

template<int N> inline builder negctrl() {
    static_assert(N >= 1, "negctrl<N> requires N >= 1");
    builder out(*this);
    for (int i = 0; i < N; ++i) {
        token tk{token::NEG_CTRL};
        out = out * builder(*this, tk);
    }
    return out;
}

private:
    void dispatch(int tgt,
                  const math::matrix_t& m,
                  const std::vector<int>& pcs,
                  const std::vector<int>& ncs) const
    {
        assert(simulator_ && "simulator not registered");
        qcs::gate_matrix(simulator_,
                         m,
                         tgt,
                         pcs.data(), static_cast<int>(pcs.size()),
                         ncs.data(), static_cast<int>(ncs.size()));
    }

    qcs::simulator* simulator_ = nullptr;
    int next_id_ = 0;
};

} // namespace qasm
