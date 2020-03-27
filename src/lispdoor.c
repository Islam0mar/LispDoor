/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "lispdoor.h"

LispObject LispApply(LispObject fun, LispObject arg_list) {
  LispObject v, ans, *arg_syms, sym, *body, *frame, frame_aux1, frame_aux2,
      *rest, label;
  LispFixNum saved_stack_ptr = stack_ptr, nargs;
  LispEnvPtr penv = LispEnv();
  /* protect from GC */
  PUSH(penv->frame);
  PUSH(fun);
  PUSH(arg_list);
  PUSH(LISP_NIL);
  frame = &stack[stack_ptr - 1];

  /* builtin func/special */
  if (LISP_CFunctionP(fun)) {
    stack[saved_stack_ptr + 1] = fun;
    v = arg_list;
    /* evaluate argument list, placing arguments on stack */
    while (LISP_ConsP(v)) {
      v = EVAL(LISP_CONS_CAR(v), penv);
      penv->frame = stack[saved_stack_ptr];
      PUSH(v);
      v = stack[saved_stack_ptr + 2] =
          LISP_CONS_CDR(stack[saved_stack_ptr + 2]);
    }
    nargs = stack_ptr - saved_stack_ptr - 4;
    fun = stack[saved_stack_ptr + 1];
    /* call function */
    ans = (fun->cfun.f)(nargs);
  } else if (LISP_ConsP(fun) &&
             (LISP_CONS_CAR(fun) == LispMakeSymbol("lambda") ||
              LISP_CONS_CAR(fun) == LispMakeSymbol("label") ||
              LISP_CONS_CAR(fun) == LispMakeSymbol("macro"))) {
    bool macro_p =
        (LISP_CONS_CAR(fun) == LispMakeSymbol("macro")) ? true : false;
    bool label_p =
        (LISP_CONS_CAR(fun) == LispMakeSymbol("label")) ? true : false;
    if (label_p) {
      /* (label name (lambda ...)) */
      label = fun;
      fun = LISP_CONS_CAR(LISP_CONS_CDR(LISP_CONS_CDR(fun)));
    }
    /* defined func */
    /* Lambda closure (lambda args body . frame) */

    v = LISP_CONS_CDR(fun);

    PUSH(LISP_CONS_CAR(v));
    arg_syms = &stack[stack_ptr - 1];
    PUSH(LISP_CONS_CAR(LISP_CONS_CDR(v)));
    body = &stack[stack_ptr - 1];
    *frame = LISP_CONS_CDR(LISP_CONS_CDR(v));
    if (LISP_ConsP(*frame)) {
      frame_aux2 = frame_aux1 = *frame;
      while (LISP_ConsP(frame_aux1)) {
        frame_aux2 = frame_aux1;
        frame_aux1 = LISP_CONS_CDR(frame_aux1);
      }
      LISP_CONS_CDR(frame_aux2) = stack[saved_stack_ptr];
    } else {
      *frame = stack[saved_stack_ptr];
    }

    if (label_p) {
      /* (label name (lambda ...)) */
      PUSH(LISP_CONS_CAR(LISP_CONS_CDR(label)));                /* name */
      PUSH(LISP_CONS_CAR(LISP_CONS_CDR(LISP_CONS_CDR(label)))); /* lambda */
      *frame = cons_(cons(stack[stack_ptr - 2], stack[stack_ptr - 1]), *frame);
      /* refetch arg_list */
      arg_list = stack[saved_stack_ptr + 2];
    }

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
      if (!macro_p) {
        v = EVAL(v, penv);
        penv->frame = stack[saved_stack_ptr];
      }
      PUSH(v);
      *frame = cons_(cons(sym, v), *frame);
      *arg_syms = LISP_CONS_CDR(*arg_syms);
      v = stack[saved_stack_ptr + 2] =
          LISP_CONS_CDR(stack[saved_stack_ptr + 2]);
    }

    /* rest args */
    if (!LISP_NULL(*arg_syms)) {
      if (LISP_SymbolP(*arg_syms)) {
        if (macro_p) {
          *frame = cons_(cons(*arg_syms, stack[saved_stack_ptr + 2]), *frame);
        } else {
          PUSH(LISP_NIL);
          PUSH(LISP_NIL);
          /* correct arg_list */
          v = stack[saved_stack_ptr + 2];
          rest = &stack[stack_ptr - 1];
          // build list of rest arguments
          // we have to build it forwards, which is tricky
          while (LISP_ConsP(v)) {
            v = LISP_CONS_CAR(v);
            v = EVAL(v, penv);
            penv->frame = stack[saved_stack_ptr]; /* old_frame */
            PUSH(v);
            v = cons_(stack[stack_ptr - 1], LISP_NIL);
            POPN(1);
            if (LISP_ConsP(*rest))
              LISP_CONS_CDR(*rest) = v;
            else
              stack[stack_ptr - 2] = v;
            *rest = v;
            v = stack[saved_stack_ptr + 2] =
                LISP_CONS_CDR(stack[saved_stack_ptr + 2]);
          }
          *frame = cons_(cons(*arg_syms, stack[stack_ptr - 2]), *frame);
        }
      } else if (LISP_ConsP(*arg_syms)) {
        LispError("apply: error: too few arguments\n");
      }
    }

    penv->frame = *frame;
    ans = EVAL(*body, penv);
    penv->frame = stack[saved_stack_ptr];
    if (macro_p) {
      ans = EVAL(ans, penv);
      penv->frame = stack[saved_stack_ptr];
    }

  } else {
    LispTypeError("apply", "lambda, macro, label or builtin", fun);
    ans = LISP_NIL;
  }
  stack_ptr = saved_stack_ptr;
  return ans;
}

LispObject EvalSexpr(LispObject expr, LispEnvPtr penv) {
  LispObject ans = LISP_NIL, v, arg_list, bind, func, *frame;
  LispFixNum saved_stack_ptr;
  /* EVAL_TOP: */
  saved_stack_ptr = stack_ptr;
  ans = LISP_UNBOUND;
  PUSH(expr);
  PUSH(penv->frame);
  frame = &stack[stack_ptr - 1];
  if (LISP_SymbolP(expr)) {
    /* Symbol */
    v = *frame;
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
      LispError("eval: error: variable %s has no value\n",
                LispSymbolName(expr));
    }
  } else if (LISP_ConsP(expr)) {
    /* Cons */
    func = LISP_CONS_CAR(expr);
    /* Apply function */
    func = EVAL(func, penv);
    penv->frame = *frame;
    arg_list = LISP_CONS_CDR(stack[saved_stack_ptr]);
    if (LISP_CFunctionP(func) && LISP_CFUNCTION_SPECIALP(func)) {
      PUSH(arg_list);
      ans = (func->cfun.f)(1);
    } else {
      ans = LispApply(func, arg_list);
    }
  } else {
    LispTypeError("eval_sexpr", "symbol or cons", expr);
  }
  stack_ptr = saved_stack_ptr;
  penv->frame = *frame;
  return ans;
}

// initialization
// -------------------------------------------------------------
void LispInit(void) {
  stack_ptr = 0;
  symbol_table_pool_here = symbol_table_pool;
  from_space = heap1;
  to_space = heap2;
  curr_heap = from_space;
  heap_limit = curr_heap + HEAP_SIZE;
  LispEnv()->symbol_table = LispMakeSymbol("t");
  LispEnv()->frame = LISP_NIL;
  LispEnv()->nvalues = 0;

  cons_flags =
      LispMakeInitializedBitVector(HEAP_SIZE / sizeof(LispSmallestStruct), 0);
  LabelTableInit(&print_conses, 32);

  LispObject o;

  LISP_SET_FUNCTION("gc", GC);
  LISP_SET_CONSTANT_VALUE("nil", LISP_NIL);
  LISP_SET_CONSTANT_VALUE("t", LISP_T);
  LISP_SET_SPECIAL("lambda", Lambda);
  LISP_SET_SPECIAL("macro", Macro);
  LISP_SET_SPECIAL("quote", Quote);
  LISP_SET_SPECIAL("'", Quote);
  LISP_SET_SPECIAL("label", Label);
  LISP_SET_SPECIAL("if", If);
  LISP_SET_SPECIAL("cond", Cond);
  LISP_SET_SPECIAL("and", And);
  LISP_SET_SPECIAL("or", Or);
  LISP_SET_SPECIAL("while", While);
  LISP_SET_SPECIAL("progn", Progn);
  LISP_SET_FUNCTION("set", Set);
  LISP_SET_FUNCTION("+", Sum);
  LISP_SET_FUNCTION("eq", Eq);
  LISP_SET_FUNCTION("sp", Sp);
  LISP_SET_FUNCTION("boundp", Boundp);
  LISP_SET_FUNCTION("cons", LispCons);
  LISP_SET_FUNCTION("car", LispCar);
  LISP_SET_FUNCTION("cdr", LispCdr);
  LISP_SET_FUNCTION("gensym", LispMakeGenSym);
}
// repl
// -----------------------------------------------------------------------
LispObject TopLevelEval(LispObject expr) {
  LispObject v;
  LispIndex saved_stack_ptr = stack_ptr;
  LispEnv()->frame = LISP_NIL;
  v = EVAL(expr, LispEnv());
  /* fixme: */
  LispEnv()->frame = LISP_NIL;
  stack_ptr = saved_stack_ptr;
  return v;
}

int main(int argc, char *argv[]) {
  LispObject expr;

  stack_bottom = ((char *)&expr) - PROCESS_STACK_SIZE;
  LispInit();
  /* if (setjmp(toplevel)) { */
  /*   SP = 0; */
  /*   fprintf(stderr, "\n"); */
  /*   if (infile) { */
  /*     fprintf(stderr, "error loading file \"%s\"\n", infile); */
  /*     infile = NULL; */
  /*   } */
  /*   goto repl; */
  /* } */
  printf("LispDoor Version: %s\n", LISP_DOOR_VERSION_STRING);
  printf("%lu live objects occupy %ld/%lu bytes.\n\n",
         *LispNumberOfObjectsAllocated(),
         (LispIndex)curr_heap - (LispIndex)from_space, HEAP_SIZE);
  setjmp(LispEnv()->top_level);

  while (1) {
    printf("> ");
    expr = ReadSexpr(stdin);
    /* LispPrint(v);  */
    /* printf("++++\n"); */
    expr = TopLevelEval(expr);
    if (feof(stdin)) {
      break;
    }
    /* LispPrint(expr); */
    LispPrint(stdout, expr, 0);
    /* printf("****\n"); */
    // print2DUtil((struct LispSymbol *)LispEnv()->symbol_table, 0);
    /* printf("----\n"); */
    /* LispPrint(v); */
    /* print(stdout, v = toplevel_EVAL(v)); */
    /* set(symbol("that"), v); */
    printf("\n\n");
  }
  return 0;
}
