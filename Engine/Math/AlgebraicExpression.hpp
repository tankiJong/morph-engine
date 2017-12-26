#pragma once

enum ExpressionOperation {
  EXP_OP_VAL = -1,
  EXP_OP_ADD,
  EXP_OP_MINUS,
  EXP_OP_MUTIPLY,
  EXP_OP_DIVIDE,
  NUM_EXP_OP,
};

class AlgebraicTerm;

class AlgebraicExpression {
public:
  virtual ~AlgebraicExpression() {
    delete m_left;
    m_left = nullptr;
    delete m_right;
    m_right = nullptr;
  };

  virtual float evaluate() const { return 0; };

  inline AlgebraicExpression*& left() { return m_left; }
  inline AlgebraicExpression*& right() { return m_right; }
  inline ExpressionOperation& op() { return m_op; }

  static AlgebraicExpression create(AlgebraicTerm left, ExpressionOperation op, AlgebraicTerm right);

  AlgebraicExpression&& operator+(const AlgebraicExpression& right) const;
  AlgebraicExpression&& operator-(const AlgebraicExpression& right) const;
  AlgebraicExpression&& operator*(const AlgebraicExpression& right) const;
  AlgebraicExpression&& operator/(const AlgebraicExpression& right) const;
protected:
  AlgebraicExpression(): m_op(EXP_OP_VAL) {} ;
  ExpressionOperation m_op;
  AlgebraicExpression* m_left = nullptr;
  AlgebraicExpression* m_right = nullptr;
};

class AlgebraicTerm: public AlgebraicExpression {
public:
  AlgebraicTerm(float value);
  float evaluate() const override {
    return m_literal;
  };

protected:
  float m_literal;
};
