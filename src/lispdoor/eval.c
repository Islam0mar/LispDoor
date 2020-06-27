/*
 *    \file eval.c
 *
 * Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 * This file is part of LispDoor.
 *
 *     LispDoor is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     LispDoor is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with LispDoor.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyrights and
 * permission notices:
 *
 *    Copyright (c) 2008 Jeff Bezanson
 *
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are
 * met:
 *
 *        * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *        * Neither the author nor the names of any contributors may be used to
 *          endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *    Copyright (c) 1984, Taiichi Yuasa and Masami Hagiya.
 *    Copyright (c) 1990, Giuseppe Attardi.
 *    Copyright (c) 2001, Juan Jose Garcia Ripoll.
 *
 *    ECL is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 */

#include "lispdoor/eval.h"

#include "lispdoor/memorylayout.h"
#include "lispdoor/print.h"
#include "lispdoor/symboltree.h"
#include "lispdoor/utils.h"

LispObject LispApply(LispObject fun, LispObject arg_list) {
  LispObject v, ans, *arg_syms, sym, *body, *frame;
  LispIndex saved_stack_index = stack_index, nargs;
  LispEnvPtr penv = LispEnv();
  /* protect from GC */
  PUSH(penv->frame);
  PUSH(arg_list);
  PUSH(LISP_NIL);
  frame = &stack[stack_index - 1];

  /* builtin func/special */
  if (LISP_CFunctionP(fun)) {
    v = arg_list;
    /* evaluate argument list, placing arguments on stack */
    while (LISP_ConsP(v)) {
      PUSH(v);
      v = LISP_CONS_CDR(v);
    }
    nargs = stack_index - saved_stack_index - 4;
    /* call function */
    ans = (fun->cfun.f)(nargs);
  } else if (LISP_ConsP(fun) && LISP_SymbolP(LISP_CONS_CAR(fun)) &&
             (strcmp(LISP_CONS_CAR(fun)->symbol.name, "lambda") == 0 ||
              strcmp(LISP_CONS_CAR(fun)->symbol.name, "macro") == 0 ||
              strcmp(LISP_CONS_CAR(fun)->symbol.name, "label") == 0)) {
    bool macro_p =
        (strcmp(LISP_CONS_CAR(fun)->symbol.name, "macro") == 0) ? true : false;

    /* defined func */
    /* Lambda closure (lambda args body . frame) */
    v = LISP_CONS_CDR(fun);

    PUSH(LISP_CONS_CAR(v));
    arg_syms = &stack[stack_index - 1];
    PUSH(LISP_CONS_CAR_SAFE(LISP_CONS_CDR(v)));
    body = &stack[stack_index - 1];
    *frame = LISP_CONS_CDR(LISP_CONS_CDR(v));

    /* 1. extend frame: bind args */
    v = arg_list;
    while (LISP_ConsP(v)) {
      if (!LISP_ConsP(*arg_syms)) {
        if (LISP_NULL(*arg_syms)) {
          LispError("apply: error: too many arguments\n");
        }
        break;
      }
      sym = LISP_CONS_CAR(*arg_syms);
      if (!LISP_SymbolP(sym)) {
        LispError("apply: error: formal argument not a symbol\n");
      }
      v = LISP_CONS_CAR(v);
      *frame = cons_(cons(sym, v), *frame);
      *arg_syms = LISP_CONS_CDR(*arg_syms);
      v = stack[saved_stack_index + 1] =
          LISP_CONS_CDR(stack[saved_stack_index + 1]);
    }
    /* rest args */
    if (!LISP_NULL(*arg_syms)) {
      if (LISP_SymbolP(*arg_syms)) {
        *frame = cons_(cons(*arg_syms, stack[saved_stack_index + 1]), *frame);
      } else if (LISP_ConsP(*arg_syms)) {
        LispError("apply: error: too few arguments\n");
      }
    }

    penv->frame = *frame;
    if (macro_p) {
      stack_index = saved_stack_index;
      PUSH(stack[saved_stack_index]);
      ans = EVAL(*body, penv);
      penv->frame = POP();
      ans = EVAL(ans, penv);
    } else {
      ans = EVAL(*body, penv);
    }

  } else {
    LispTypeError("apply", "lambda, macro, label or builtin", fun);
    ans = LISP_NIL;
  }
  penv->frame = stack[saved_stack_index];
  stack_index = saved_stack_index;
  return ans;
}

LispObject EvalSexpr(LispObject expr, LispEnvPtr penv) {
  LispObject ans, v, arg_list, bind, func, *frame;
  LispIndex saved_stack_index;
EVAL_TOP:
  if ((Byte *)&ans < stack_bottom) {
    LispError("eval: error: c-stack overflow\n");
  }

  saved_stack_index = stack_index;
  ans = LISP_UNBOUND;
  PUSH(expr);
  PUSH(penv->frame);
  PUSH(LISP_NIL);
  frame = &stack[stack_index - 1];
  if (LISP_SymbolP(expr)) {
    /* Symbol */
    v = penv->frame;

    while (LISP_ConsP(v)) {
      bind = LISP_CONS_CAR(v);
      if (LISP_ConsP(bind) && LISP_CONS_CAR(bind) == expr) {
        ans = LISP_CONS_CDR(bind);
        break;
      }
      v = LISP_CONS_CDR(v);
    }

    if (LISP_UNBOUNDP(ans)) {
      ans = expr->symbol.value;
    }
    if (LISP_UNBOUNDP(ans)) {
      LispPrintStr("eval: error: variable ");
      LispPrintStr(LispSymbolName(expr));
      LispError(" has no value\n");
    }
  } else if (LISP_ConsP(expr)) {
    func = LISP_CONS_CAR(expr);
    /* eval function */
    func = EVAL(func, penv);
    penv->frame = stack[saved_stack_index + 1];

    stack[saved_stack_index] = arg_list =
        LISP_CONS_CDR(stack[saved_stack_index]);
    if (LISP_CFunctionP(func) && LISP_CFUNCTION_SPECIALP(func)) {
      PUSH(arg_list);
      ans = (func->cfun.f)(1);
    } else {
      /* Apply */
      LispIndex nargs;
      LispObject *arg_syms, sym, *body, *rest, *fun;
      fun = &stack[stack_index];
      PUSH(func);
      nargs = stack_index;
      /* builtin func/special */
      if (LISP_CFunctionP(func)) {
        v = stack[saved_stack_index];
        /* evaluate argument list, placing arguments on stack */
        while (LISP_ConsP(v)) {
          v = EVAL(LISP_CONS_CAR(v), penv);
          penv->frame = stack[saved_stack_index + 1];
          PUSH(v);
          v = stack[saved_stack_index] =
              LISP_CONS_CDR(stack[saved_stack_index]);
        }
        nargs = stack_index - nargs;
        /* call function */
        ans = ((*fun)->cfun.f)(nargs);
      } else if (LISP_ConsP(func) && LISP_SymbolP(LISP_CONS_CAR(func)) &&
                 (strcmp(LISP_CONS_CAR(func)->symbol.name, "lambda") == 0 ||
                  strcmp(LISP_CONS_CAR(func)->symbol.name, "macro") == 0 ||
                  strcmp(LISP_CONS_CAR(func)->symbol.name, "label") == 0)) {
        bool macro_p = (strcmp(LISP_CONS_CAR(func)->symbol.name, "macro") == 0)
                           ? true
                           : false;

        /* defined func */
        /* Lambda closure (lambda args body . frame) */
        v = LISP_CONS_CDR(func);

        PUSH(LISP_CONS_CAR_SAFE(v));
        arg_syms = &stack[stack_index - 1];
        PUSH(LISP_CONS_CAR_SAFE(LISP_CONS_CDR(v)));
        body = &stack[stack_index - 1];
        *frame = LISP_CONS_CDR(LISP_CONS_CDR(v));

        /* 1. extend frame: bind args */
        v = stack[saved_stack_index];
        while (LISP_ConsP(v)) {
          if (!LISP_ConsP(*arg_syms)) {
            if (LISP_NULL(*arg_syms)) {
              LispError("apply: error: too many arguments\n");
            }
            break;
          }
          sym = LISP_CONS_CAR(*arg_syms);
          if (!LISP_SymbolP(sym)) {
            LispError("apply: error: formal argument not a symbol\n");
          }
          v = LISP_CONS_CAR(v);
          if (!macro_p) {
            v = EVAL(v, penv);
            penv->frame = stack[saved_stack_index + 1];
          }
          *frame = cons_(cons(sym, v), *frame);
          *arg_syms = LISP_CONS_CDR(*arg_syms);
          v = stack[saved_stack_index] =
              LISP_CONS_CDR(stack[saved_stack_index]);
        }
        /* rest args */
        if (!LISP_NULL(*arg_syms)) {
          if (LISP_SymbolP(*arg_syms)) {
            if (macro_p) {
              *frame = cons_(cons(*arg_syms, stack[saved_stack_index]), *frame);
            } else {
              PUSH(LISP_NIL);
              PUSH(LISP_NIL);
              /* correct arg_list */
              v = stack[saved_stack_index];
              rest = &stack[stack_index - 1];
              // build list of rest arguments
              // we have to build it forwards, which is tricky
              while (LISP_ConsP(v)) {
                v = LISP_CONS_CAR(v);
                v = EVAL(v, penv);
                penv->frame = stack[saved_stack_index + 1]; /* old_frame */
                v = cons_(v, LISP_NIL);
                if (LISP_ConsP(*rest)) {
                  LISP_CONS_CDR(*rest) = v;
                } else {
                  stack[stack_index - 2] = v;
                }
                *rest = v;
                v = stack[saved_stack_index] =
                    LISP_CONS_CDR(stack[saved_stack_index]);
              }
              *frame = cons_(cons(*arg_syms, stack[stack_index - 2]), *frame);
            }
          } else if (LISP_ConsP(*arg_syms)) {
            LispError("apply: error: too few arguments\n");
          }
        }
        penv->frame = *frame;
        if (macro_p) {
          stack_index = saved_stack_index;
          PUSH(stack[saved_stack_index + 1]);
          ans = EVAL(*body, penv);
          penv->frame = POP();
          TAIL_EVAL(ans, penv);
        } else {
          TAIL_EVAL(*body, penv);
        }
      } else {
        LispTypeError("apply", "lambda, macro, label or builtin", *fun);
        ans = LISP_NIL;
      }
    }
  } else {
    LispTypeError("eval_sexpr", "symbol or cons", expr);
  }
  stack_index = saved_stack_index;
  penv->frame = stack[saved_stack_index + 1];
  return ans;
}

/* repl */
LispObject TopLevelEval(LispObject expr) {
  LispObject v;
  LispIndex saved_stack_index = stack_index;
  LispEnv()->frame = LISP_NIL;
  v = EVAL(expr, LispEnv());
  LispEnv()->frame = LISP_NIL;
  stack_index = saved_stack_index;
  return v;
}
