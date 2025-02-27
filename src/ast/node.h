// Author: Caden LeCluyse

#ifndef NODE_H
#define NODE_H

#include <gmpxx.h>
#include <stdexcept>
#include <string>
#include <memory>
#include <mpfr.h>

struct BoolNode {
    explicit BoolNode(const char token) noexcept;
    virtual ~BoolNode() = default;
    [[nodiscard]] virtual bool evaluate() const = 0;
    std::unique_ptr<BoolNode> m_left_child;
    std::unique_ptr<BoolNode> m_right_child;
    const char key;
};

struct ValueBNode : public BoolNode {
    explicit ValueBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct OperationBNode : public BoolNode {
    explicit OperationBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct UnaryBNode : public BoolNode {
    explicit UnaryBNode(const char token) : BoolNode(token) {}
    [[nodiscard]] bool evaluate() const override;
};

struct MathNode {
    MathNode() = default;
    virtual ~MathNode() = default;
    [[nodiscard]] virtual mpz_class evaluate() const = 0;
    virtual mpfr_t& evaluate_float() = 0;
    std::unique_ptr<MathNode> m_left_child;
    std::unique_ptr<MathNode> m_right_child;
};

struct ValueMNode : public MathNode {
    explicit ValueMNode(const std::string& _value_mpz, const std::string& _value_mpf) :
        value_mpz(_value_mpz) {
        mpfr_init2(value_mpfr, 256);
        const int successful = mpfr_set_str(value_mpfr, _value_mpf.c_str(), 10, MPFR_RNDN);
        if (successful != 0) {
            mpfr_clear(value_mpfr);
            throw std::invalid_argument("Invalid floating point: " + _value_mpf);
        }
    }
    ~ValueMNode() { mpfr_clear(value_mpfr); }

    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpz_class value_mpz;
    mpfr_t value_mpfr;
};

struct OperationMNode : public MathNode {
    explicit OperationMNode(const char token) : key(token) {
        mpfr_init2(node_result, 256);
    }
    ~OperationMNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
    const char key;
};

struct FactorialNode : public MathNode {
    FactorialNode() {
        mpfr_init2(node_result, 256);
    }
    ~FactorialNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
};

struct UnaryMNode : public MathNode {
    explicit UnaryMNode() {
        mpfr_init2(node_result, 256);
    }
    ~UnaryMNode() { mpfr_clear(node_result); }
    [[nodiscard]] mpz_class evaluate() const override;
    mpfr_t& evaluate_float() override;
    mpfr_t node_result;
};

#endif
