#pragma once
#include <vector>
#include <cassert>

#include "math_type.hpp"
#include "qcs.hpp"

namespace qasm
{

    struct slice_t
    {
        int first;
        int last;
    };

    slice_t slice(int first, int last);

    struct set
    {
        std::vector<int> indices;
        set(std::initializer_list<int> lst);
    };

    struct indices_t
    {
        std::vector<int> values;
    };

    class qubits;
    class clbits;
    struct token;
    class builder;

    class qasm
    {
    public:
        inline qasm() = default;
        inline virtual ~qasm() = default;

        /*-------------------------------------------------------
         * qubits allocation helper
         *------------------------------------------------------*/
        qubits qalloc(int n);
        clbits clalloc(int n);

        /*-------------------------------------------------------
         * 外部 Simulator 登録
         *------------------------------------------------------*/
        void register_simulator(qcs::simulator *sim) noexcept;

        /*-------------------------------------------------------
         * 単一量子ゲート生成関数
         *------------------------------------------------------*/
        builder h();
        builder u(double th, double ph, double la);
        builder cu(math::complex_t m00, math::complex_t m01, math::complex_t m10, math::complex_t m11);
        builder pow(double exp);
        builder inv();
        builder sqrt();

        // user-defined circuit to be overridden
        virtual void circuit();

        /*-------------------------------------------------------
         * 条件付き演算子（N ビット版）
         *------------------------------------------------------*/
        builder ctrl(int N = 1);
        builder negctrl(int N = 1);

        template <typename... Q>
        void reset(Q... qs)
        {
            std::vector<int> argv;
            append_args(argv, qs...);
            assert(simulator_ && "simulator not registered");
            for (int q : argv)
            {
                simulator_->reset(q);
            }
        }

        template <typename... Q>
        void measure(Q... qs)
        {
            std::vector<int> argv;
            append_args(argv, qs...);
            assert(simulator_ && "simulator not registered");
            for (int q : argv)
            {
                simulator_->measure(q);
            }
        }

    private:
        void dispatch(int tgt, const math::matrix_t &m, const std::vector<int> &pcs, const std::vector<int> &ncs, bool u4) const;

        static void append_arg(std::vector<int> &out, int v);
        static void append_arg(std::vector<int> &out, const indices_t &idx);
        static void append_arg(std::vector<int> &out, const qubits &qs);
        static void append_args(std::vector<int> &);
        template <typename First, typename... Rest>
        static void append_args(std::vector<int> &out, First &&first, Rest &&...rest)
        {
            append_arg(out, std::forward<First>(first));
            append_args(out, std::forward<Rest>(rest)...);
        }

        qcs::simulator *simulator_ = nullptr;
        int next_id_ = 0;
        int next_clbit_id_ = 0;
        friend class builder;
        friend class qubits;
        friend class clbits;
    };

    /*-------------------------------------------------------
     * 量子レジスタ（連番 ID を払い出すだけ）
     *------------------------------------------------------*/
    class qubits
    {
    public:
        qubits(qasm &ctx, int n);
        int operator[](int i) const;
        indices_t operator[](slice_t sl) const;
        indices_t operator[](const set &s) const;
        indices_t operator[](std::initializer_list<int> lst) const;

    private:
        qasm &ctx_;
        int base_;
        int size_;
        friend class qasm;
    };

    /*-------------------------------------------------------
     * 古典レジスタ
     *------------------------------------------------------*/
    class clbits
    {
    public:
        clbits(qasm &ctx, int n);
        int operator[](int i) const;

    private:
        qasm &ctx_;
        int base_;
        int size_;
    };

    /*-------------------------------------------------------
     * ビルダーに詰めるトークン
     *------------------------------------------------------*/
    struct token
    {
        enum kind_t
        {
            POS_CTRL,
            NEG_CTRL,
            MATRIX,
            U4,
            POW,
            INV
        } kind;
        math::matrix_t mat{};
        double val = 1;
        explicit token(kind_t k) : kind(k), mat(), val(1) {}
    };

    /*-------------------------------------------------------
     * ゲート式ビルダー
     *------------------------------------------------------*/
    class builder
    {
    public:
        builder(const qasm &ctx);
        builder(const qasm &ctx, token tk);
        builder(const builder &rhs);
        builder &operator=(const builder &rhs);

        builder operator*(const builder &rhs) const;

        template <typename... Q>
        void operator()(Q... qs) const
        {
            static_assert(sizeof...(qs) > 0, "at least one qubit is required");
            std::vector<int> argv;
            argv.reserve(sizeof...(qs));
            append_args(argv, qs...);
            (*this)(argv);
        }

        void operator()(const std::vector<int> &argv) const;

        explicit builder(token tk) = delete;

    private:
        const qasm &ctx_;
        std::vector<token> seq_;

        static void append_arg(std::vector<int> &out, int v);
        static void append_arg(std::vector<int> &out, const indices_t &idx);
        static void append_args(std::vector<int> &);
        template <typename First, typename... Rest>
        static void append_args(std::vector<int> &out, First &&first, Rest &&...rest)
        {
            append_arg(out, std::forward<First>(first));
            append_args(out, std::forward<Rest>(rest)...);
        }

        void dispatch(int tgt, const math::matrix_t &m, const std::vector<int> &pcs, const std::vector<int> &ncs, bool u4) const;
    };

} // namespace qasm
