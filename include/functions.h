/**
 *   \file functions.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "eval.h"
#include "memorylayout.h"
#include "objects.h"
#include "print.h"
#include "symboltree.h"
#include "utils.h"

/* helper functions */
LispObject LispApply(LispObject fun, LispObject arg_list);

// Builtin functions
// -------------------------------------------------------------
LispObject LdLabel(LispNArg narg) {
  LispObject v, name, body;
  LispFixNum saved_stack_ptr;
  ArgCount("label", narg, 1);
  /* the syntax of label is (label name (lambda args body ...)) */
  v = POP();
  saved_stack_ptr = stack_ptr;
  name = LISP_CONS_CAR_SAFE(v); /* name */
  PUSH(name);
  body = LISP_CONS_CAR_SAFE(LISP_CONS_CDR(v)); /* lambda expr */
  body = EVAL(body, LispEnv());                /* evaluate lambda */
  PUSH(body);
  name = stack[saved_stack_ptr];
  /* (lambda args body . frame) */
  v = LISP_CONS_CDR_SAFE(LISP_CONS_CDR_SAFE(LISP_CONS_CDR_SAFE(body)));
  if (LISP_ConsP(v)) {
    v = cons_(cons(name, body), v);
  } else {
    v = cons_(cons(name, body), LISP_NIL);
  }
  body = stack[saved_stack_ptr + 1];
  LISP_CONS_CDR(LISP_CONS_CDR(LISP_CONS_CDR(body))) = v;
  return body;
}
LispObject LdLambda(LispNArg narg) {
  LispObject v, arg_syms, body, ans;
  ArgCount("lambda", narg, 1);
  /* build a closure (lambda args body . frame) */
  v = POP();
  arg_syms = LISP_CONS_CAR_SAFE(v);
  body = LISP_CONS_CAR_SAFE(LISP_CONS_CDR(v));
  ans = cons_(LispMakeSymbol("lambda"),
              cons(arg_syms, cons(body, LispEnv()->frame)));
  return ans;
}
LispObject LdMacro(LispNArg narg) {
  LispObject v, arg_syms, body, ans;
  ArgCount("macro", narg, 1);
  /* build a closure (lambda args body . frame) */
  v = POP();
  arg_syms = LISP_CONS_CAR_SAFE(v);
  body = LISP_CONS_CAR_SAFE(LISP_CONS_CDR(v));
  ans = cons_(LispMakeSymbol("macro"),
              cons(arg_syms, cons(body, LispEnv()->frame)));
  return ans;
}
LispObject LdQuote(LispNArg narg) {
  LispObject ans, v;
  v = POP();
  if (!LISP_ConsP(v)) {
    LispError("quote: error: expected argument\n");
  }
  ans = LISP_CONS_CAR(v);
  return ans;
}
LispObject LdIf(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  LispObject ans, cond;
  PUSH(LispEnv()->frame);
  cond = LISP_CONS_CAR_SAFE(stack[stack_ptr - 2]);
  if (LISP_NULL(EVAL(cond, LispEnv()))) {
    ans = LISP_CONS_CDR_SAFE(LISP_CONS_CDR(stack[stack_ptr - 2]));
    if (LISP_ConsP(ans)) {
      ans = LISP_CONS_CAR(ans);
    } else {
      ans = LISP_NIL;
    }
  } else {
    ans = LISP_CONS_CAR_SAFE(LISP_CONS_CDR(stack[stack_ptr - 2]));
  }
  LispEnv()->frame = stack[stack_ptr - 1];
  ans = EVAL(ans, LispEnv());
  LispEnv()->frame = stack[stack_ptr - 1];
  /* POPN(2); */
  return ans;
}
LispObject LdCond(LispNArg narg) {
  /* (cond   (test1    action1) */
  /*    (test2    action2) */
  /*    ... */
  /*    (testn   actionn))*/
  LispObject ans = LISP_NIL, *pv, v, *frame;
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  pv = &stack[stack_ptr - 2];
  while (LISP_ConsP(*pv)) {
    v = EVAL(ToCons(LISP_CONS_CAR(*pv), "cond")->car, LispEnv());
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
LispObject LdAnd(LispNArg narg) {
  LispObject ans = LISP_T, *pv, *frame;
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
LispObject LdOr(LispNArg narg) {
  LispObject ans = LISP_NIL, *pv, *frame;
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
LispObject LdWhile(LispNArg narg) {
  /* (while test body ...) */
  LispObject *tmp, *pv, *frame, *body, *cond;
  tmp = &stack[stack_ptr - 1];
  PUSH(LispEnv()->frame);
  frame = &stack[stack_ptr - 1];
  PUSH(LISP_CONS_CDR_SAFE(*tmp));
  body = &stack[stack_ptr - 1];
  PUSH(LISP_CONS_CAR(*tmp));
  cond = &stack[stack_ptr - 1];
  PUSH(LISP_NIL);
  pv = &stack[stack_ptr - 1];
  while (!LISP_NULL(EVAL(*cond, LispEnv()))) {
    LispEnv()->frame = *frame;
    *tmp = *body;
    while (LISP_ConsP(*tmp)) {
      *pv = EVAL(LISP_CONS_CAR(*tmp), LispEnv());
      LispEnv()->frame = *frame;
      *tmp = LISP_CONS_CDR(*tmp);
    }
  }
  return *pv;
}
LispObject LdProgn(LispNArg narg) {
  /* return last arg */
  LispObject ans = LISP_NIL, *frame, *body;
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
LispObject LdEq(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  ArgCount("eq", narg, 2);
  /* POPN(2); */
  return LISP_MAKE_BOOL(stack[stack_ptr - 1] == stack[stack_ptr - 2]);
}

LispObject LdSet(LispNArg narg) {
  LispObject ans, v, e, bind;
  ArgCount("set", narg, 2);
  bool done = false;
  ans = POP();
  e = POP();
  v = LispEnv()->frame;
  while (LISP_ConsP(v)) {
    bind = LISP_CONS_CAR(v);
    if (LISP_ConsP(bind) && LISP_CONS_CAR(bind) == e) {
      LISP_CONS_CDR(bind) = ans;
      done = true;
    }
    v = LISP_CONS_CDR(v);
  }
  if (!done) {
    ToSymbol(e, "set")->value = ans;
  }

  return ans;
}
LispObject LdBoundp(LispNArg narg) {
  /* (if (test-clause) (action1) (action2)) */
  ArgCount("boundp", narg, 1);
  LispObject v = ToSymbol(stack[stack_ptr - 1], "boundp")->value;
  return LISP_MAKE_BOOL(!LISP_UNBOUNDP(v));
}
LispObject LdCons(LispNArg narg) {
  ArgCount("cons", narg, 2);
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = stack[stack_ptr - 2];
  LISP_CONS_CDR(c) = stack[stack_ptr - 1];
  return c;
}
LispObject LdCar(LispNArg narg) {
  ArgCount("car", narg, 1);
  return LISP_CONS_CAR_SAFE(stack[stack_ptr - 1]);
}

LispObject LdCdr(LispNArg narg) {
  ArgCount("cdr", narg, 1);
  return LISP_CONS_CDR_SAFE(stack[stack_ptr - 1]);
}

LispObject LdRPlacA(LispNArg narg) {
  ArgCount("rplaca", narg, 2);
  LISP_CONS_CAR_SAFE(stack[stack_ptr - 2]) = stack[stack_ptr - 1];
  return stack[stack_ptr - 2];
}
LispObject LdRPlacD(LispNArg narg) {
  ArgCount("rplacd", narg, 2);
  LISP_CONS_CDR_SAFE(stack[stack_ptr - 2]) = stack[stack_ptr - 1];
  return stack[stack_ptr - 2];
}
LispObject LdAtom(LispNArg narg) {
  ArgCount("atom", narg, 1);
  return LISP_MAKE_BOOL(LISP_ATOM(stack[stack_ptr - 1]));
}
LispObject LdConsP(LispNArg narg) {
  ArgCount("consp", narg, 1);
  return LISP_MAKE_BOOL(LISP_ConsP(stack[stack_ptr - 1]));
}
LispObject LdSymbolP(LispNArg narg) {
  ArgCount("symbolp", narg, 1);
  return LISP_MAKE_BOOL(LISP_SymbolP(stack[stack_ptr - 1]));
}
LispObject LdNumberP(LispNArg narg) {
  ArgCount("numberp", narg, 1);
  return LISP_MAKE_BOOL(LISP_NumberP(stack[stack_ptr - 1]));
}
LispObject LdFixNumP(LispNArg narg) {
  ArgCount("fixnump", narg, 1);
  return LISP_MAKE_BOOL(LISP_FixNumP(stack[stack_ptr - 1]));
}
LispObject LdAdd(LispNArg narg) {
  LispFixNum ans = 0;
  LispIndex i = stack_ptr - narg;
  for (; i < stack_ptr; ++i) {
    ans += ToFixNum(stack[i], "+");
  }
  return LISP_MAKE_FIXNUM(ans);
}
LispObject LdSub(LispNArg narg) {
  if (narg < 1) {
    LispError("-: error: too few arguments\n");
  }
  LispIndex i = stack_ptr - narg;
  LispFixNum ans = (narg == 1) ? 0 : ToFixNum(stack[i++], "-");
  for (; i < stack_ptr; ++i) {
    ans -= ToFixNum(stack[i], "-");
  }
  return LISP_MAKE_FIXNUM(ans);
}
LispObject LdMul(LispNArg narg) {
  LispIndex i = stack_ptr - narg;
  LispFixNum ans = 1;
  for (; i < stack_ptr; ++i) {
    ans *= ToFixNum(stack[i], "*");
  }
  return LISP_MAKE_FIXNUM(ans);
}
LispObject LdDiv(LispNArg narg) {
  if (narg < 1) {
    LispError("-: error: too few arguments\n");
  }
  LispIndex i = stack_ptr - narg;
  LispFixNum ans = (narg == 1) ? 1 : ToFixNum(stack[i++], "/");
  LispFixNum tmp;
  for (; i < stack_ptr; ++i) {
    tmp = ToFixNum(stack[i], "*");
    if (tmp == 0) {
      LispError("/: error: division by zero\n");
    }
    ans /= tmp;
  }
  return LISP_MAKE_FIXNUM(ans);
}
LispObject LdLt(LispNArg narg) {
  ArgCount("<", narg, 2);
  // this implements generic comparison for all atoms
  // strange comparisons (for example with builtins) are resolved
  // arbitrarily but consistently.
  // ordering: cons < builtin < number < symbol
  LispObject o1, o2, ans = LISP_NIL;
  o1 = stack[stack_ptr - 2];
  o2 = stack[stack_ptr - 1];
  if (LISP_TYPE_OF(o1) != LISP_TYPE_OF(o2)) {
    ans = ((LISP_TYPE_OF(o1) < LISP_TYPE_OF(o2)) ? LISP_T : LISP_NIL);
  } else {
    switch (LISP_TYPE_OF(o1)) {
      case kList: {
        LispError("<: error: expected number type or symbol\n");
        break;
      }
      case kCharacter:
      case kFixNum: {
        ans = LISP_MAKE_BOOL(o1 < o2);
        break;
      }
      case kSingleFloat: {
        ans = LISP_MAKE_BOOL(o1->single_float.value < o2->single_float.value);
        break;
      }
      case kDoubleFloat: {
        ans = LISP_MAKE_BOOL(o1->double_float.value < o2->double_float.value);
        break;
      }
      case kLongFloat: {
        ans = LISP_MAKE_BOOL(o1->long_float.value < o2->long_float.value);
        break;
      }
      case kSymbol: {
        ans = LISP_MAKE_BOOL(strcmp(o1->symbol.name, o2->symbol.name));
        break;
      }

      case kCFunction: {
        ans = LISP_MAKE_BOOL(strcmp(o1->cfun.name, o2->cfun.name));
        break;
      }
      case kBitVector: {
        LispError("<: error: expected number type or symbol\n");
        break;
      }
      case kString: {
        ans = LISP_MAKE_BOOL(strcmp(o1->string.self, o2->string.self));
        break;
      }
      case kGenSym: {
        ans = LISP_MAKE_BOOL(o1->gen_sym.id < o2->gen_sym.id);
        break;
      }
      case kVector: {
        LispError("<: error: expected number type or symbol\n");
        break;
      }
      default:
        printf("\ttype = %d\n", LISP_TYPE_OF(o1));
        LispError("alloc botch.\n");
        break;
    }
  }
  return ans;
}

LispObject LdNot(LispNArg narg) {
  ArgCount("fixnump", narg, 1);
  return LISP_MAKE_BOOL(stack[stack_ptr - 1] == LISP_NIL);
}
LispObject LdEval(LispNArg narg) {
  ArgCount("eval", narg, 1);
  return EVAL(stack[stack_ptr - 1], LispEnv());
}
LispObject LdPrint(LispNArg narg) {
  ArgCount("print", narg, 1);
  LispIndex i = stack_ptr - narg;
  for (; i < stack_ptr; ++i) {
    LispPrint(stdout, stack[i], 0);
  }
  fprintf(stdout, "\n");
  return stack[stack_ptr - 1];
}
LispObject LdPrinc(LispNArg narg) {
  ArgCount("princ", narg, 1);
  LispIndex i = stack_ptr - narg;
  for (; i < stack_ptr; ++i) {
    LispPrint(stdout, stack[i], 1);
  }
  fprintf(stdout, "\n");
  return stack[stack_ptr - 1];
}
LispObject LdRead(LispNArg narg) {
  ArgCount("read", narg, 0);
  return ReadSexpr(stdin);
}
LispObject LdLoad(LispNArg narg) {
  ArgCount("load", narg, 1);
  return LoadFile(ToSymbol(stack[stack_ptr - 1], "load")->name);
}
LispObject LdExit(LispNArg narg) {
  ArgCount("exit", narg, 0);
  exit(0);
  return LISP_NIL;
}
LispObject LdError(LispNArg narg) {
  LispIndex i = stack_ptr - narg;
  for (; i < stack_ptr; ++i) {
    LispPrint(stderr, stack[i], 1);
  }
  LispError("\n");
  return LISP_NIL;
}
LispObject LdProg1(LispNArg narg) {
  if (narg < 1) {
    LispError("prog1: error: too few arguments\n");
  }
  return stack[stack_ptr - narg];
}
LispObject LdAssoc(LispNArg narg) {
  ArgCount("assoc", narg, 2);
  LispObject bind, v = stack[stack_ptr - 2], item = stack[stack_ptr - 1];
  LispObject ans = LISP_NIL;
  while (LISP_ConsP(v)) {
    bind = LISP_CONS_CAR(v);
    if (LISP_ConsP(bind) && LISP_CONS_CAR(bind) == item) {
      ans = bind;
      break;
    }
    v = LISP_CONS_CDR(v);
  }
  return ans;
}
LispObject LdApply(LispNArg narg) {
  ArgCount("apply", narg, 2);
  LispObject v = stack[stack_ptr - 1], f = stack[stack_ptr - 2];
  /* POPN(2); */
  if (LISP_CFunctionP(f) && LISP_CFUNCTION_SPECIALP(f)) {
    LispError(
        "apply: error: cannot apply special operator "
        "%s\n",
        f->cfun.name);
  }
  return LispApply(f, v);
}
/* usefull */
LispObject LdPrintStack(LispNArg narg) {
  LispFixNum i = stack_ptr;
  ArgCount("print-stack", narg, 0);
  printf("\nstack_ptr: %ld\n", stack_ptr);
  printf("stack: %p\n", stack);
  printf("stack_b: %p\n", stack_bottom);
  printf("heap_l: %p\n", heap_limit);
  printf("heap: %p\n", curr_heap);
  while (i >= 0) {
    printf("%ld:", i);
    LispPrint(stdout, stack[i], 0);
    printf("\n");
    i--;
  }
  return LISP_T;
}
LispObject LdResetStack(LispNArg narg) {
  ArgCount("reset-stack", narg, 0);
  stack_ptr = 0;
  LispPrint(stdout, LISP_T, 0);
  printf("\n");
  longjmp(LispEnv()->top_level, 1);
  return LISP_T;
}
#endif /* FUNCTIONS_H */
