#include "Source/Float.h"
#include "Lang.h"  // only for assign()!

namespace V3DLib {

// ============================================================================
// Class FloatExpr
// ============================================================================

FloatExpr::FloatExpr(float x) {
  m_expr = std::make_shared<Expr>(x);
}

FloatExpr::FloatExpr(Deref<Float> d) : BaseExpr(d.expr()) {}


// ============================================================================
// Class Float
// ============================================================================

Float::Float() {
  Var v  = freshVar();
  m_expr = mkVar(v);
}


Float::Float(float x) {
  Var v  = freshVar();
  m_expr = mkVar(v);
  auto a = std::make_shared<Expr>(x);
  assign(m_expr, a);
}


Float::Float(FloatExpr e) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, e.expr());
}


Float::Float(Deref<Float> d) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, d.expr());
}


// Copy constructors

Float::Float(Float &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}


Float::Float(Float const &x) {
  Var v    = freshVar();
  m_expr = mkVar(v);
  assign(m_expr, x.expr());
}


/**
 * Cast to an FloatExpr
 */
Float::operator FloatExpr() { return FloatExpr(m_expr); }


/**
 * Assignment
 */
Float &Float::operator=(Float &rhs) {
  assign(m_expr, rhs.expr());
  return rhs;
}


FloatExpr Float::operator=(FloatExpr rhs) {
  assign(m_expr, rhs.expr());
  return rhs;
}


Float &Float::operator+=(FloatExpr rhs) {
  *this = *this + rhs;
  return *this;
}


// ============================================================================
// Generic operations
// ============================================================================

inline FloatExpr mkFloatApply(FloatExpr lhs, Op op, FloatExpr rhs) {
  Expr::Ptr e = mkApply(lhs.expr(), op, rhs.expr());
  return FloatExpr(e);
}


inline FloatExpr mkFloatApply(FloatExpr rhs, Op op) {
  Expr::Ptr e = mkApply(rhs.expr(), op);
  return FloatExpr(e);
}


// ============================================================================
// Specific operations
// ============================================================================

// Read an Float from the UNIFORM FIFO.
FloatExpr getUniformFloat() {
  Expr::Ptr e = mkVar(UNIFORM);
  return FloatExpr(e);
}


// Read vector from VPM
FloatExpr vpmGetFloat() {
  Expr::Ptr e = mkVar(VPM_READ);
  return FloatExpr(e);
}


FloatExpr operator+(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(ADD, FLOAT), b); }
FloatExpr operator-(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(SUB, FLOAT), b); }
FloatExpr operator*(FloatExpr a, FloatExpr b) { return mkFloatApply(a, Op(MUL, FLOAT), b); }
FloatExpr min(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MIN, FLOAT), b); }
FloatExpr max(FloatExpr a, FloatExpr b)       { return mkFloatApply(a, Op(MAX, FLOAT), b); }

// SFU functions
FloatExpr recip(FloatExpr x)     { return mkFloatApply(x, Op(RECIP, FLOAT)); }
FloatExpr recipsqrt(FloatExpr x) { return mkFloatApply(x, Op(RECIPSQRT, FLOAT)); }
FloatExpr exp(FloatExpr x)       { return mkFloatApply(x, Op(EXP, FLOAT)); }
FloatExpr log(FloatExpr x)       { return mkFloatApply(x, Op(LOG, FLOAT)); }

}  // namespace V3DLib
