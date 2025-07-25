#pragma once
#include <vector>
#include <array>
#include <complex>
#include <cassert>
#include <cstddef>

#include "qcs.hpp"

namespace qasm {

/*-------------------------------------------------------
 * 0.  外部 Simulator 登録
 *------------------------------------------------------*/
inline qcs::simulator* g_simulator = nullptr;

inline void register_simulator(qcs::simulator* sim) noexcept {
    g_simulator = sim;
}

/*-------------------------------------------------------
 * 1.  量子レジスタ（連番 ID を払い出すだけ）
 *------------------------------------------------------*/
namespace detail {
    inline int g_next_id = 0;   // グローバル連番
}

class qubits {
public:
    explicit qubits(int n) : base_(detail::g_next_id), size_(n) {
        assert(n > 0);
        detail::g_next_id += n;
        qcs::alloc_qubit(n);    // 低レイヤへ通知
    }
    int operator[](int i) const {
        assert(0 <= i && i < size_);
        return base_ + i;
    }
private:
    int base_;
    int size_;
};

/*-------------------------------------------------------
 * 2.  ビルダーに詰めるトークン
 *------------------------------------------------------*/
struct token {
    enum kind_t { POS_CTRL, NEG_CTRL, MATRIX, POW, INV } kind;
    qcs::matrix_t mat {};   // MATRIX のときに使用
    double        val = 1;  // POW のときに使用
};

/*-------------------------------------------------------
 * 3.  ゲート式ビルダー
 *------------------------------------------------------*/
class builder {
public:
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
        std::array<int, sizeof...(qs)> argv{qs...};
        std::vector<int> pos_ctrls, neg_ctrls;
        qcs::matrix_t     mat {};
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
                pow_exp = t.val; break;
            case token::INV:
                invert = true; break;
            case token::MATRIX:
                mat = t.mat;     // 基本行列
                // apply optional operations
                if (pow_exp != 1.0) {
                    mat = qcs::matrix_pow(mat, pow_exp);
                }
                if (invert) {
                    mat = qcs::matrix_inv(mat);
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
    explicit builder(token tk) { seq_.push_back(tk); }
    builder() = default;

private:
    std::vector<token> seq_;

    static void dispatch(int tgt,
                         const qcs::matrix_t& m,
                         const std::vector<int>& pcs,
                         const std::vector<int>& ncs)
    {
        assert(g_simulator && "simulator not registered");
        qcs::gate_matrix(g_simulator,
                         m,
                         tgt,
                         pcs.data(), static_cast<int>(pcs.size()),
                         ncs.data(), static_cast<int>(ncs.size()));
    }
};

/*-------------------------------------------------------
 * 4.  単一量子ゲート生成関数
 *------------------------------------------------------*/
inline builder h() {
    token tk{token::MATRIX};
    tk.mat = qcs::matrix_hadamard;
    return builder{tk};
}

inline builder u(double th, double ph, double la) {
    token tk{token::MATRIX};
    tk.mat = qcs::generate_matrix_u(th, ph, la);
    return builder{tk};
}

inline builder pow(double exp) {
    token tk{token::POW};
    tk.val = exp;
    return builder{tk};
}

inline builder inv() {
    token tk{token::INV};
    return builder{tk};
}

/*-------------------------------------------------------
 * 5.  条件付き演算子（1 ビット版のみ）
 *------------------------------------------------------*/
template<int N> inline builder ctrl() {
    static_assert(N == 1, "only ctrl<1> is supported in this minimal impl.");
    token tk{token::POS_CTRL};
    return builder{tk};
}

template<int N> inline builder negctrl() {
    static_assert(N == 1, "only negctrl<1> is supported in this minimal impl.");
    token tk{token::NEG_CTRL};
    return builder{tk};
}

} // namespace qasm
