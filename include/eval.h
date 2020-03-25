#ifndef EVAL_H
#define EVAL_H

#include "objects.h"

LispObject TopLevelEval(LispObject expr);
LispObject EvalSexpr(LispObject expr, LispEnvPtr penv);
#define EVAL(expr, env) \
  ((LISP_ATOM(expr) && !LISP_SymbolP(expr)) ? (expr) : EvalSexpr((expr), env))
#define TAIL_EVAL(xpr, env)                         \
  do {                                              \
    stack_ptr = saved_stack_ptr;                    \
    if ((LISP_ATOM(expr) && !LISP_SymbolP(expr))) { \
      return (xpr);                                 \
    } else {                                        \
      expr = (xpr);                                 \
      penv = (env);                                 \
      goto EVAL_TOP;                                \
    }                                               \
  } while (0)

#endif /* EVAL_H */
