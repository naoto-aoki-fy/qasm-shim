#pragma once
#include <vector>
#include <cassert>

namespace qcs{
    class simulator;
}

namespace qasm
{

    struct slice_t
    {
        int first;
        int last;

        struct iterator
        {
            int value;
            explicit iterator(int v) : value(v) {}
            int operator*() const { return value; }
            iterator &operator++() { ++value; return *this; }
            bool operator!=(const iterator &other) const { return value != other.value; }
        };

        iterator begin() const { return iterator(first); }
        iterator end() const { return iterator(last + 1); }
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
    class bit;
    struct token;
    class builder;

    class qasm
    {
    public:
        inline qasm() = default;
        inline virtual ~qasm() = default;

        /*-------------------------------------------------------
         * qubits / bits allocation helper
         *------------------------------------------------------*/
        qubits qalloc(int n);
        bit clalloc(int n);

        /*-------------------------------------------------------
         * 外部 Simulator 登録
         *------------------------------------------------------*/
        void register_simulator(qcs::simulator *sim) noexcept;

        /*-------------------------------------------------------
         * 単一量子ゲート生成関数
         *------------------------------------------------------*/
        builder h();
        builder x();
        builder u(double th, double ph, double la);
        builder cu(double th, double ph, double la, double ga);
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

        /*-------------------------------------------------------
         * reset / measure helper
         *------------------------------------------------------*/
        void reset(const qubits &qs);
        void reset(const indices_t &qs);
        std::vector<int> measure(const qubits &qs);
        std::vector<int> measure(const indices_t &qs);

    private:
        qcs::simulator *simulator_ = nullptr;
        int next_id_ = 0;
        friend class builder;
        friend class qubits;
    };

    /*-------------------------------------------------------
     * 古典ビット
     *------------------------------------------------------*/
    class bit
    {
    public:
        bit() = default;
        explicit bit(int n) : values_(n) {}

        struct slice_proxy
        {
            bit &ref;
            slice_t sl;
            slice_proxy(bit &r, slice_t s) : ref(r), sl(s) {}
            slice_proxy &operator=(const std::vector<int> &vals)
            {
                int len = sl.last - sl.first + 1;
                for (int i = 0; i < len && i < (int)vals.size(); ++i)
                {
                    ref.values_[sl.first + i] = vals[i];
                }
                return *this;
            }
        };
        slice_proxy operator[](slice_t sl)
        {
            assert(0 <= sl.first && sl.first <= sl.last && sl.last < (int)values_.size());
            return slice_proxy(*this, sl);
        }

        struct indices_proxy
        {
            bit &ref;
            std::vector<int> idx;
            indices_proxy(bit &r, std::vector<int> i) : ref(r), idx(std::move(i)) {}
            indices_proxy &operator=(const std::vector<int> &vals)
            {
                for (std::size_t i = 0; i < idx.size() && i < vals.size(); ++i)
                {
                    ref.values_[idx[i]] = vals[i];
                }
                return *this;
            }
        };
        indices_proxy operator[](const set &s)
        {
            for (int v : s.indices)
            {
                assert(0 <= v && v < (int)values_.size());
            }
            return indices_proxy(*this, s.indices);
        }
        indices_proxy operator[](std::initializer_list<int> lst)
        {
            std::vector<int> idx(lst);
            for (int v : idx)
            {
                assert(0 <= v && v < (int)values_.size());
            }
            return indices_proxy(*this, std::move(idx));
        }

        bit &operator=(const std::vector<int> &vals)
        {
            for (std::size_t i = 0; i < values_.size() && i < vals.size(); ++i)
            {
                values_[i] = vals[i];
            }
            return *this;
        }

    private:
        std::vector<int> values_;
        friend class qasm;
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
     * ビルダーに詰めるトークン
     *------------------------------------------------------*/
    struct token
    {
        enum kind_t
        {
            POS_CTRL,
            NEG_CTRL,
            POW,
            INV,
            HADAMARD,
            X,
            U4
        } kind;
        double theta = 0;
        double phi = 0;
        double lambda = 0;
        double gamma = 0;
        double val = 1;
        explicit token(kind_t k) : kind(k) {}
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

    };

} // namespace qasm
