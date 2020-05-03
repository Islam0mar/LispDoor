/**
 *   \file eval.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_EVAL_H_INCLUDED
#define LISPDOOR_EVAL_H_INCLUDED

#include "lispdoor/objects.h"

#define EVAL(expr, env) \
  ((LISP_ATOM(expr) && !LISP_SymbolP(expr)) ? (expr) : EvalSexpr((expr), env))
#define TAIL_EVAL(xpr, env)                         \
  do {                                              \
    stack_index = saved_stack_index;                \
    if ((LISP_ATOM(expr) && !LISP_SymbolP(expr))) { \
      return (xpr);                                 \
    } else {                                        \
      expr = (xpr);                                 \
      penv = (env);                                 \
      goto EVAL_TOP;                                \
    }                                               \
  } while (0)
LispObject LispApply(LispObject fun, LispObject arg_list);
LispObject EvalSexpr(LispObject expr, LispEnvPtr penv);
LispObject TopLevelEval(LispObject expr);
LispObject EvalSexpr(LispObject expr, LispEnvPtr penv);

#endif /* LISPDOOR_EVAL_H_INCLUDED */
