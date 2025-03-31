// Author: Caden LeCluyse

#ifndef MNODE_H
#define MNODE_H

#include <gmpxx.h>
#include <string_view>
#include <memory>
#include <mpfr.h>

#include "include/types.hpp"
#include "startup/startup.h"

using namespace Types;

namespace MathNodes {

// According to the MPFR docs, when using c++ you should try to avoid making copies whenever possible,
// so evaluate_float returns a reference to a node_result, which is initialized in the constructor of the node
struct MathNode {
    MathNode() = default;
    virtual ~MathNode() = default;
    [[nodiscard]] virtual mpz_class evaluate() const = 0;
    virtual mpfr_t& evaluate_float() = 0;
    std::unique_ptr<MathNode> m_left_child;
    std::unique_ptr<MathNode> m_right_child;
};

struct ValueMNode : public MathNode {
    explicit ValueMNode(const std::string_view _value_mpz, const std::string_view _value_mpf);
    explicit ValueMNode(const Token token);
    ~ValueMNode() {
        mpfr_clear(value_mpfr);
        mpfr_free_cache();
    }

    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpz_class value_mpz;
    mpfr_t value_mpfr;
};

struct OperationMNode : public MathNode {
    explicit OperationMNode(const Token token) : key(token) {
        mpfr_init2(node_result, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    }
    ~OperationMNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
    const Token key;
};

struct TrigMNode : public MathNode {
    explicit TrigMNode(const Token token) : key(token) {
        mpfr_init2(node_result, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    }
    ~TrigMNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override{ return 0; }
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
    const Token key;
};

struct FactorialNode : public MathNode {
    explicit FactorialNode() {
        mpfr_init2(node_result, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    }
    ~FactorialNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
};

struct UnaryMNode : public MathNode {
    explicit UnaryMNode() {
        mpfr_init2(node_result, static_cast<mpfr_prec_t>(Startup::settings.at(Setting::PRECISION)));
    }
    ~UnaryMNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
};

}

#endif
