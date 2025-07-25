// ─────────────────────────────────────────────────────────────
// qsim.hpp  —  出現順対応版  (C++11)
// ─────────────────────────────────────────────────────────────
#pragma once
#include <vector>
#include <cstddef>
#include <cassert>

// ------------------------------------------------------------
// C API (アプリ側実装)
// ------------------------------------------------------------
struct simulator;
extern void declare_num_qubit(int);
extern void gate_h(simulator*, int, int const*, int, int const*, int);
extern void gate_u(simulator*, double, double, double,
                   int, int const*, int, int const*, int);

// ------------------------------------------------------------
// グローバルシミュレータ
// ------------------------------------------------------------
inline simulator* g_sim = nullptr;

inline void call_h(int tgt,
                   const std::vector<int>& pc,
                   const std::vector<int>& nc)
{
    gate_h(g_sim, tgt,
           const_cast<int*>(pc.data()), static_cast<int>(pc.size()),
           const_cast<int*>(nc.data()), static_cast<int>(nc.size()));
}
inline void call_u(double th,double ph,double la,int tgt,
                   const std::vector<int>& pc,
                   const std::vector<int>& nc)
{
    gate_u(g_sim, th, ph, la, tgt,
           const_cast<int*>(pc.data()), static_cast<int>(pc.size()),
           const_cast<int*>(nc.data()), static_cast<int>(nc.size()));
}

// ------------------------------------------------------------
// qubits
// ------------------------------------------------------------
class qubits{
public:
    explicit qubits(int n):first_(alloc(n)),size_(n)
    { declare_num_qubit(total_); }

    int operator[](int i) const { assert(i<size_); return first_+i; }

private:
    int first_,size_;
    static int total_;
    static int alloc(int n){ int b=total_; total_+=n; return b; }
};
int qubits::total_ = 0;

// ------------------------------------------------------------
// タグ類
// ------------------------------------------------------------
template<int N,bool IS_NEG>
struct ctrl_tag{
    static constexpr int count = N;
    static constexpr bool neg  = IS_NEG;
};
template<int N> using ctrl    = ctrl_tag<N,false>;
template<int N> using negctrl = ctrl_tag<N,true>;

struct pow_tag{ double e; explicit constexpr pow_tag(double d):e(d){} };
constexpr pow_tag pow(double d){ return pow_tag{d}; }

// ------------------------------------------------------------
// 単一量子ゲート
// ------------------------------------------------------------
struct h_gate{
    void operator()(double, int tgt,
                    const std::vector<int>& pc,
                    const std::vector<int>& nc) const
    { call_h(tgt,pc,nc); }
};
constexpr h_gate h(){ return {}; }

struct u_gate{
    double th,ph,la;
    constexpr u_gate(double t,double p,double l):th(t),ph(p),la(l){}
    void operator()(double pw,int tgt,
                    const std::vector<int>& pc,
                    const std::vector<int>& nc) const
    { call_u(th*pw,ph*pw,la*pw,tgt,pc,nc); }
};
constexpr u_gate u(double t,double p,double l){ return {t,p,l}; }

// ダミーゲート
struct no_gate{
    void operator()(double, int,
                    const std::vector<int>&,
                    const std::vector<int>&) const {}
};

// ------------------------------------------------------------
// expr  ―  登場順パターンを保持
// ------------------------------------------------------------
template<typename Gate,int POS,int NEG>
class expr{
public:
    expr(Gate g,double pw,std::vector<bool> pat)
        :gate_(g),power_(pw),pattern_(std::move(pat)){}

    double power() const { return power_; }
    const Gate& gate() const { return gate_; }
    const std::vector<bool>& pattern() const { return pattern_; }

    template<typename... Q>
    void operator()(Q... q) const {
        // コンパイル時：テンプレート引数で個数を確認
        static_assert(sizeof...(q) == POS + NEG + 1,
                      "ctrl(+neg)+target の数が一致しません");
        // 実行時：パターン長が POS+NEG とずれていないか確認（理論上同じになるはず）
        assert(pattern_.size() == POS + NEG && "パターン長が不正");
        int a[] = { static_cast<int>(q)... };
        std::vector<int> pc, nc;
        pc.reserve(POS); nc.reserve(NEG);

        for(std::size_t i = 0; i < pattern_.size(); ++i){
            (pattern_[i]? nc : pc).push_back(a[i]);
        }
        int tgt = a[pattern_.size()];
        gate_(power_,tgt,pc,nc);
    }
private:
    Gate                gate_;
    double              power_;
    std::vector<bool>   pattern_;          // true=negctrl, false=ctrl
};

// ------------------------------------------------------------
// ヘルパー: パターン生成＆連結
// ------------------------------------------------------------
inline std::vector<bool> make_pattern(int n,bool neg){
    return std::vector<bool>(n, neg);
}
inline std::vector<bool> concat(std::vector<bool> a,const std::vector<bool>& b){
    a.insert(a.end(), b.begin(), b.end()); return a;
}

// ------------------------------------------------------------
// operator* オーバーロード
// ------------------------------------------------------------
template<int N,bool NG,typename Gate>
inline auto operator*(ctrl_tag<N,NG>, Gate g){
    return expr<Gate,
                NG?0:N,
                NG?N:0>{g,1.0, make_pattern(N,NG)};
}

template<typename Gate>
inline auto operator*(pow_tag pw, Gate g){
    return expr<Gate,0,0>{g,pw.e, {}};
}

template<int N,bool NG>
inline auto operator*(ctrl_tag<N,NG>, pow_tag pw){
    return expr<no_gate,
                NG?0:N,
                NG?N:0>{no_gate{},pw.e, make_pattern(N,NG)};
}

template<typename Gate,int P,int N>
inline auto operator*(expr<Gate,P,N> e, pow_tag pw){
    return expr<Gate,P,N>{e.gate(), e.power()*pw.e, e.pattern()};
}

template<typename Gate0,int P,int N,typename Gate1>
inline auto operator*(expr<Gate0,P,N> e, Gate1 g){
    return expr<Gate1,P,N>{g, e.power(), e.pattern()};
}

template<typename Gate,int P,int N,int M>
inline auto operator*(expr<Gate,P,N> e, ctrl_tag<M,false>){
    return expr<Gate, P+M, N>{e.gate(), e.power(),
                              concat(e.pattern(), make_pattern(M,false))};
}
template<typename Gate,int P,int N,int M>
inline auto operator*(expr<Gate,P,N> e, ctrl_tag<M,true>){
    return expr<Gate, P, N+M>{e.gate(), e.power(),
                              concat(e.pattern(), make_pattern(M,true))};
}
template<int M,bool NG,typename Gate,int P,int N>
inline auto operator*(ctrl_tag<M,NG> c, expr<Gate,P,N> e){
    return e * c;
}

template<int M,bool NG1,int N,bool NG2>
inline auto operator*(ctrl_tag<M,NG1>, ctrl_tag<N,NG2>){
    return expr<no_gate,
                (NG1?0:M)+(NG2?0:N),
                (NG1?M:0)+(NG2?N:0)>
           {no_gate{},1.0,
            concat(make_pattern(M,NG1), make_pattern(N,NG2))};
}
