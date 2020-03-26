/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "memorylayout.h"
#include "objects.h"
#include "print.h"
#include "eval.h"
#include "symboltree.h"
/* helper functions */
static inline LispIndex ConsCount(LispObject v) {
  LispIndex nargs = 0;
  while (LISP_ConsP(v)) {
    nargs++;
    v = LISP_CONS_CDR(v);
  }
  return nargs;
}
static inline LispObject ConsNth(LispIndex n, LispObject lst) {
  LispIndex i = 0;
  for (; i < n; ++i) {
    lst = LISP_CONS_CDR(lst);
  }
  lst = LISP_CONS_CAR(lst);
  return lst;
}

// Builtin functions
// -------------------------------------------------------------
LispObject Sum(LispNArg narg) {
  LispFixNum ans = 0;
  LispIndex i = stack_ptr - narg;
  for (; i < stack_ptr; ++i) {
    ans += LISP_FIXNUM(stack[i]);
  }
  return LISP_MAKE_FIXNUM(ans);
}
LispObject Label(LispNArg narg) {
  LispObject v, *pv, *frame, *body;
  /* the syntax of label is (label name (lambda args body ...)) */
  v = POP();
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(v)); /* name */
  pv = &stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(LISP_CONS_CDR(v))); /* function */
  body = &stack[stack_ptr - 1];
  *body = EVAL(*body, LispEnv()); /* evaluate lambda */
  v = cons_(LispMakeSymbol("label"), cons(*pv, cons(*body, LISP_NIL)));
  return v;
}
LispObject Lambda(LispNArg narg) {
  LispObject v, arg_syms, env, body, ans;
  /* build a closure (lambda args body . frame) */
  v = POP();
  PUSH(LISP_CONS_CAR(v));
  arg_syms = stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(LISP_CONS_CDR(v)));
  body = stack[stack_ptr - 1];
  ans = cons_(LispMakeSymbol("lambda"),
              cons(arg_syms, cons(body, LispEnv()->frame)));

  POPN(2);
  return ans;
}
LispObject Macro(LispNArg narg) {
  LispObject v, arg_syms, env, body, ans;
  /* build a closure (lambda args body . frame) */
  v = POP();
  PUSH(LISP_CONS_CAR(v));
  arg_syms = stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(LISP_CONS_CDR(v)));
  body = stack[stack_ptr - 1];
  ans = cons_(LispMakeSymbol("macro"),
              cons(arg_syms, cons(body, LispEnv()->frame)));

  POPN(2);
  return ans;
}
/* LispObject sp(LispNArg narg) { */
/*   LispFixNum i = stack_ptr; */
/*   /\* Lambda/macro *\/ */
/*   argcount("macro", narg, 0); */
/*   LispObject v, arg_syms, env, body, ans; */
/*   /\* build a closure (lambda args body . frame) *\/ */
/*   printf("\nstack_ptr: %ld\n", stack_ptr); */
/*   printf("stack: %p\n", stack); */
/*   printf("stack_b: %p\n", stack_bottom); */
/*   printf("heap_l: %p\n", heap_limit); */
/*   printf("heap: %p\n", curr_heap); */
/*   while (i >= 0) { */
/*     printf("%ld:", i); */
/*     LispPrint(stack[i]); */
/*     printf("\n"); */
/*     i--; */
/*   } */

/*   return LISP_NIL; */
/* } */
LispObject Quote(LispNArg narg) {
  LispObject ans, v;
  v = POP();
  if (!LISP_ConsP(v)) {
    LispError("quote: error: expected argument\n");
  }
  ans = LISP_CONS_CAR(v);
  return ans;
}
LispObject If(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  LispObject ans, cond;
  PUSH(LispEnv()->frame);
  cond = LISP_CONS_CAR(stack[stack_ptr - 2]);
  if (LISP_NULL(EVAL(cond, LispEnv()))) {
    /* FIXME: (if nil 1) */
    ans = LISP_CONS_CAR(LISP_CONS_CDR(LISP_CONS_CDR(stack[stack_ptr - 2])));
  } else {
    ans = LISP_CONS_CAR(LISP_CONS_CDR(stack[stack_ptr - 2]));
  }
  LispEnv()->frame = stack[stack_ptr - 1];
  ans = EVAL(ans, LispEnv());
  LispEnv()->frame = stack[stack_ptr - 1];
  POPN(2);
  return ans;
}
LispObject Cond(LispNArg narg) {
  /* (cond   (test1    action1) */
  /*    (test2    action2) */
  /*    ... */
  /*    (testn   actionn))*/
  LispObject ans = LISP_NIL, *pv, v, *frame, *body;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  pv = &stack[stack_ptr - 2];
  while (LISP_ConsP(*pv)) {
    v = LISP_CONS_CAR(*pv);
    if (!LISP_ConsP(v)) {
      LispTypeError("cond", "cons", v);
    }
    v = EVAL(LISP_CONS_CAR(v), LispEnv());
    LispEnv()->frame = *frame;
    if (!LISP_NULL(v)) {
      *pv = LISP_CONS_CDR(LISP_CONS_CAR(*pv));
      /* evaluate body forms */
      if (LISP_ConsP(*pv)) {
        while (LISP_ConsP(LISP_CONS_CDR(*pv))) {
          v = EVAL(LISP_CONS_CAR(*pv), LispEnv());
          LispEnv()->frame = *frame;
          *pv = LISP_CONS_CDR(*pv);
        }
        ans = EVAL(LISP_CONS_CAR(*pv), LispEnv());
        LispEnv()->frame = *frame;
      }
      break;
    }
    *pv = LISP_CONS_CDR(*pv);
  }
  return ans;
}
LispObject And(LispNArg narg) {
  LispObject ans = LISP_T, *pv, *frame, *body;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  pv = &stack[stack_ptr - 2];
  if (LISP_ConsP(*pv)) {
    while (LISP_ConsP(LISP_CONS_CDR(*pv))) {
      if ((EVAL(LISP_CONS_CAR(*pv), LispEnv())) == LISP_NIL) {
        ans = LISP_NIL;
        break;
      }
      LispEnv()->frame = *frame;
      *pv = LISP_CONS_CDR(*pv);
    }
    ans = LISP_NULL(ans) ? ans : EVAL(LISP_CONS_CAR(*pv), LispEnv());
  }
  return ans;
}
LispObject Or(LispNArg narg) {
  LispObject ans = LISP_NIL, *pv, *frame, *body;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  pv = &stack[stack_ptr - 2];
  if (LISP_ConsP(*pv)) {
    while (LISP_ConsP(LISP_CONS_CDR(*pv))) {
      if ((EVAL(LISP_CONS_CAR(*pv), LispEnv())) != LISP_NIL) {
        ans = LISP_T;
        break;
      }
      LispEnv()->frame = *frame;
      *pv = LISP_CONS_CDR(*pv);
    }
    ans = LISP_NULL(ans) ? EVAL(LISP_CONS_CAR(*pv), LispEnv()) : ans;
  }
  return ans;
}
LispObject While(LispNArg narg) {
  /* (while test body ...) */
  LispObject ans = LISP_NIL, *pv, *frame, *body, *cond;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  pv = &stack[stack_ptr - 2];
  PUSH(LISP_CONS_CDR(*pv));
  body = &stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(*pv));
  cond = &stack[stack_ptr - 1];
  PUSH(LISP_NIL);
  pv = &stack[stack_ptr - 1];
  while (!LISP_NULL(EVAL(*cond, LispEnv()))) {
    LispEnv()->frame = *frame;
    while (LISP_ConsP(*body)) {
      *pv = EVAL(LISP_CONS_CAR(*body), LispEnv());
      LispEnv()->frame = *frame;
      *body = LISP_CONS_CDR(*body);
    }
  }
  ans = *pv;
  return ans;
}
LispObject Progn(LispNArg narg) {
  /* return last arg */
  LispObject ans = LISP_NIL, *frame, *body, *cond;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  body = &stack[stack_ptr - 2];
  if (LISP_ConsP(*body)) {
    while (LISP_ConsP(LISP_CONS_CDR(*body))) {
      ans = EVAL(LISP_CONS_CAR(*body), LispEnv());
      LispEnv()->frame = *frame;
      *body = LISP_CONS_CDR(*body);
    }
    ans = EVAL(LISP_CONS_CAR(*body), LispEnv());
    LispEnv()->frame = *frame;
  }
  return ans;
}
/* normal functions  */
LispObject Eq(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  argcount("eq", narg, 2);
  POPN(2);
  return LISP_MAKE_BOOL(stack[stack_ptr] == stack[stack_ptr + 1]);
}

LispObject Set(LispNArg narg) {
  LispObject ans, v, e, bind;
  argcount("set", narg, 2);
  ans = POP();
  v = POP();
  /* FixMe: (set x 10) */
  if (!LISP_SymbolP(v)) {
    LispError("set: error: expected symbol\n");
  }
  e = v;
  v = LispEnv()->frame;
  while (LISP_ConsP(v)) {
    bind = LISP_CONS_CAR(v);
    if (LISP_ConsP(bind) && LISP_CONS_CAR(bind) == e) {
      LISP_CONS_CDR(bind) = ans;
      return ans;
    }
    v = LISP_CONS_CDR(v);
  }
  e->symbol.value.obj = ans;
  return ans;
}
LispObject Boundp(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  argcount("boundp", narg, 1);
  POP();
  return LISP_MAKE_BOOL(!LISP_UNBOUNDP(stack[stack_ptr]));
}
LispObject LispCons(LispNArg narg) {
  argcount("cons", narg, 2);
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = stack[stack_ptr - 2];
  LISP_CONS_CDR(c) = stack[stack_ptr - 1];
  return c;
}
LispObject LispCar(LispNArg narg) {
  argcount("car", narg, 1);
  if (!LISP_ConsP(stack[stack_ptr - 1])) {
    LispTypeError("car", "cons", stack[stack_ptr - 1]);
  };
  return LISP_CONS_CAR(stack[stack_ptr - 1]);
}

LispObject LispCdr(LispNArg narg) {
  argcount("cdr", narg, 1);
  /* if (!LISP_ConsP(stack[stack_ptr - 1])) { */
  /*   LispTypeError("cdr", "cons", stack[stack_ptr - 1]); */
  /* }; */
  return ToCons(stack[stack_ptr - 1], "cdr")->cdr;
}

#endif /* FUNCTIONS_H */
