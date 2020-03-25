/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

/*
  TODO:
  - finish data types
  - add prmitive types

 */
/* combile command
   gcc -Wextra -ggdb3 -O0 -Wl,--no-undefined -Wl,--no-allow-shlib-undefined
   lispdoor.c gc.c objects.c print.c memorylayout.c read.c utils.c
   symboltree.c -lm -o lispdoor
 */

#include "lispdoor.h"

// read
// -----------------------------------------------------------------------

/* Lexical analyzer         input string --> tokens */
// read
// -----------------------------------------------------------------------

/* Printing */

// eval
// -----------------------------------------------------------------------
LispObject LispApply(LispObject fun, LispObject arg_list) {
  LispObject v, bind, ans, *arg_syms, sym, *body, *frame, frame_aux1,
      frame_aux2, *rest, label;
  LispFunc f;
  LispFixNum saved_stack_ptr = stack_ptr, nargs;
  LispEnvPtr penv = LispEnv(), lenv;
  /* protect from GC */
  PUSH(penv->frame);
  PUSH(fun);
  PUSH(arg_list);
  PUSH(LISP_NIL);
  frame = &stack[stack_ptr - 1];

  /* builtin func */
  if (LISP_BuiltInP(fun)) {
    /* FixMe: */
    fun = LISP_BUILTIN_TO_SYM(fun);
    stack[saved_stack_ptr + 1] = fun;
    if (LISP_SYMBOL_GENSYMP(fun)) {
      LispError("apply: error first arg is gensym");
    }
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
    ans = (fun->symbol.value.func)(nargs);
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
            POP();
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
  }
  stack_ptr = saved_stack_ptr;
  return ans;
}

LispObject EvalSexpr(LispObject expr, LispEnvPtr penv) {
  LispObject ans = LISP_NIL, v, aux, arg_list, bind, func, arg_syms, *frame,
             body;
  LispFixNum saved_stack_ptr, nargs;
EVAL_TOP:
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
      if (LISP_SYMBOL_BUILTINP(expr)) {
        ans = LISP_SYM_TO_BUILTIN(expr);
      } else {
        ans = expr->symbol.value.obj;
      }
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
    bool builtin_special_p = false;
    if (LISP_BuiltInP(func)) {
      v = LISP_BUILTIN_TO_SYM(func);
      builtin_special_p = LISP_SYMBOL_SPECIALP(v);
      func = builtin_special_p ? v : func;
    }
    if (LISP_SYMBOL_SPECIALP(func) || builtin_special_p) {
      PUSH(arg_list);
      ans = (func->symbol.value.func)(1);
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
  int i;

  stack_ptr = 0;
  symbol_table_pool_here = symbol_table_pool;
  from_space = heap1;
  to_space = heap2;
  curr_heap = from_space;
  heap_limit = curr_heap + HEAP_SIZE;
  LispEnv()->symbol_table = LispMakeSymbol("t");
  LispEnv()->frame = LISP_NIL;
  LispEnv()->nvalues = 0;

  cons_flags = LispMakeBitVector(HEAP_SIZE / sizeof(struct LispCons));
  LabelTableInit(&print_conses, 32);

  /* ugly! */
  /* LISP_T = LispMakeSymbol("t"); */
  LISP_SET_CONSTANT(LispMakeSymbol("nil"), LISP_NIL);
  LISP_SET_CONSTANT(LispMakeSymbol("t"), LISP_T);
  /* LISP_SET_CONSTANT(LISP_T, LISP_T); */
  LISP_SET_FUNC(LispMakeSymbol("+"), Sum);
  LISP_SET_SPECIAL(LispMakeSymbol("lambda"), Lambda);
  LISP_SET_SPECIAL(LispMakeSymbol("macro"), Macro);
  LISP_SET_SPECIAL(LispMakeSymbol("quote"), Quote);
  LISP_SET_SPECIAL(LispMakeSymbol("'"), Quote);
  LISP_SET_SPECIAL(LispMakeSymbol("label"), Label);
  LISP_SET_SPECIAL(LispMakeSymbol("if"), If);
  LISP_SET_SPECIAL(LispMakeSymbol("cond"), Cond);
  LISP_SET_SPECIAL(LispMakeSymbol("and"), And);
  LISP_SET_SPECIAL(LispMakeSymbol("or"), Or);
  LISP_SET_SPECIAL(LispMakeSymbol("while"), While);
  LISP_SET_SPECIAL(LispMakeSymbol("progn"), Progn);
  LISP_SET_FUNC(LispMakeSymbol("set"), Set);
  LISP_SET_FUNC(LispMakeSymbol("eq"), Eq);
  LISP_SET_FUNC(LispMakeSymbol("boundp"), Boundp);
  LISP_SET_FUNC(LispMakeSymbol("cons"), LispCons);
  LISP_SET_FUNC(LispMakeSymbol("car"), LispCar);
  LISP_SET_FUNC(LispMakeSymbol("cdr"), LispCdr);
  LISP_SET_FUNC(LispMakeSymbol("gensym"), LispMakeGenSym);
  /* LISP_SET_FUNC(LispMakeSymbol("sp"), sp); */
  /* LISP_SET_Lambda(LispMakeSymbol("lambda"), LISP_NIL); */
  /* TODO: */
  /* for (i = 0; i < (int)N_BUILTINS; i++) */
  /*   LISP_SET(LispMakeSymbol(builtin_names[i]), builtin(i)); */
  /* LISP_SET(LispMakeSymbol("princ"), builtin(F_PRINT)); */
}
// repl
// -----------------------------------------------------------------------

static char *infile = NULL;

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
  printf(
      "Welcome to femtoLisp "
      "----------------------------------------------------------\n");
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
