#include "AlgebraicExpression.hpp"

AlgebraicExpression AlgebraicExpression::create(AlgebraicTerm left, ExpressionOperation op, AlgebraicTerm right) {
  AlgebraicExpression exp;

  exp.m_left = new AlgebraicTerm(left);
  exp.m_right = new AlgebraicTerm(right);

  exp.m_op = op;

  return exp;
}

AlgebraicTerm::AlgebraicTerm(float value): m_literal(value) {
  m_left = nullptr;
  m_right = nullptr;
}
