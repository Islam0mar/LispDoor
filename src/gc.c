/**
 *   \file gc.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#include "gc.h"

#include "memorylayout.h"
#include "print.h"
#include "symboltree.h"
#include "utils.h"

/* if lispobject is forwarded:
   1. kForwarded is set
   2. forward pointer is set after type value
*/

LispIndex *LispNumberOfObjectsAllocated() {
  static LispIndex objects = 0;
  return &objects;
}

/* Data allocation */
void *GcMalloc(LispIndex num_of_bytes) {
  void *ptr;
  if (curr_heap + num_of_bytes > heap_limit) {
    GC(0);
  }
  if (curr_heap + num_of_bytes > heap_limit) {
    LispError("no space to allocate new object.");
  }
  ptr = (void *)curr_heap;
  curr_heap += num_of_bytes;
  curr_heap =
      (Byte *)(((LispFixNum)curr_heap + (ALIGN_BITS - 1)) & -ALIGN_BITS);
  ++*LispNumberOfObjectsAllocated(); /* better readability */
  return ptr;
}

LispObject LispAllocObject(LispType t, LispIndex extra_size) {
  static LispObject obj;
  switch (t) {
    case kList:
      obj = (LispObject)GcMalloc(sizeof(struct LispCons));
      return LISP_PTR_CONS(obj);
    case kCharacter:
      return LISP_MAKE_CHARACTER(955); /* Immediate character */
    case kFixNum:
      return LISP_MAKE_FIXNUM(0); /* Immediate fixnum */
    case kSingleFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispSingleFloat));
      break;
    case kDoubleFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispDoubleFloat));
      break;
    case kLongFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispLongFloat));
      break;
    case kSymbol:
      obj = (LispObject)SymMalloc(sizeof(struct LispSymbol) +
                                  extra_size * sizeof(char) / sizeof(Byte));
      break;
    case kCFunction:
      obj = (LispObject)GcMalloc(sizeof(struct LispCFunction));
      break;
    case kBitVector:
      obj = (LispObject)GcMalloc(sizeof(struct LispBitVector) + extra_size);
      break;
    case kString:
      obj = (LispObject)GcMalloc(sizeof(struct LispString) +
                                 extra_size * sizeof(char) / sizeof(Byte));
      break;
    case kGenSym:
      t = kSymbol;
      obj = (LispObject)GcMalloc(sizeof(struct LispGenSym));
      break;
    case kVector:
      obj =
          (LispObject)GcMalloc(sizeof(struct LispVector) +
                               sizeof(LispObject) / sizeof(Byte) * extra_size);
      break;
    default:
      printf("\ttype = %d\n", t);
      LispError("alloc botch.\n");
  }
  obj->d.t = t;
  return obj;
}
/* Symbol allocaction */
void *SymMalloc(LispIndex num_of_bytes) {
  void *ptr = symbol_table_pool_here;
  symbol_table_pool_here += num_of_bytes;
  symbol_table_pool_here =
      (Byte *)(((LispFixNum)symbol_table_pool_here + (ALIGN_BITS - 1)) &
               -ALIGN_BITS);
  if (symbol_table_pool_here > symbol_table_pool + SYMBOL_TABLE_SIZE) {
    LispError("symbol table overflow.");
  }
  return ptr;
}

// collector
// ------------------------------------------------------------------
LispObject LispGcRelocate(LispObject o) {
  LispObject o_new = o;
  LispIndex size = 0;
  if (LISP_UNBOUNDP(o)) {
  } else if (LISP_NULL(o)) {
  } else if (o == LISP_T) {
  } else {
    LispType t = LISP_TYPE_OF(o);
    switch (t) {
      case kCharacter:
      case kFixNum: {
        break;
      }
      case kForwarded: {
        o_new = LISP_FORWARD(o);
        break;
      }
      case kSymbol: {
        if (LISP_SYMBOL_GENSYMP(o)) {
          o_new = LdMakeGenSym(0);
          o_new->gen_sym.id = o->gen_sym.id; /* useful for debugging */
          LISP_SET_FORWARDED(o, o_new);
        }
        break;
      }
      case kSingleFloat: {
        o_new = LispMakeSingleFloat(o->single_float.value);
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kDoubleFloat: {
        o_new = LispMakeDoubleFloat(o->double_float.value);
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kLongFloat: {
        o_new = LispMakeLongFloat(o->long_float.value);
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kCFunction: {
        if (LISP_CFUNCTION_SPECIALP(o)) {
          o_new = LispMakeCFunctionSpecial(o->cfun.name, o->cfun.f);
        } else {
          o_new = LispMakeCFunction(o->cfun.name, o->cfun.f);
        }
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kBitVector: {
        size = o->bit_vector.size;
        o_new = LispMakeBitVectorExactSize(size);
        memcpy(o_new->bit_vector.self, o->bit_vector.self, size);
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kVector: {
        size = o->vector.size;
        o_new = LispMakeVector(size);
        o_new->vector.fillp = o->vector.fillp;
        LISP_SET_FORWARDED(o, o_new); /* o->vector.self won't be touched */
        LispIndex i = 0;
        for (i = 0; i < size; ++i) {
          o_new->vector.self[i] = LispGcRelocate(o->vector.self[i]);
        }
        break;
      }
      case kString: {
        o_new = LispMakeString(o->string.self);
        LISP_SET_FORWARDED(o, o_new);
        break;
      }
      case kList: {
        LispObject a, d, nc, *pcdr;
        bool forwarded_p = false;
        // iterative implementation allows arbitrarily long cons chains
        pcdr = &o_new;
        do {
          /* forwarded cons */
          a = LISP_CONS_CAR(o);
          if (a == LISP_UNBOUND) {
            d = LISP_CONS_CDR(o);
            forwarded_p = true;
            break;
          }
          *pcdr = nc = MakeCons();
          d = LISP_CONS_CDR(o);
          LISP_CONS_CAR(o) = LISP_UNBOUND;
          LISP_CONS_CDR(o) = nc;
          LISP_CONS_CAR(nc) = LispGcRelocate(a);
          pcdr = &LISP_CONS_CDR(nc);
          o = d;
        } while (LISP_ConsP(o));
        if (!forwarded_p) {
          d = LispGcRelocate(d);
        }
        *pcdr = d;
        break;
      }
      default:
        LispTypeError("LispGcRelocate", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
        assert(false);
    }
  }
  return o_new;
}

void TraceGlobals(struct LispSymbol *root) {
  while (root != NULL) {
    /* dfs */
    root->value = LispGcRelocate(root->value);
    TraceGlobals(root->left);
    root = root->right;
  }
}

LispObject GC(LispNArg narg) {
  Byte *temp;
  LispIndex i;
  ReadState *rs;
  LispObject *items;

  curr_heap = to_space;
  heap_limit = curr_heap + HEAP_SIZE;
  /* trace number of objects allocated */
  *LispNumberOfObjectsAllocated() = 0;
  /* 1. stack values */
  printf("gc1.stack = %ld\n", stack_ptr);
  for (i = 0; i < stack_ptr; i++) {
    stack[i] = LispGcRelocate(stack[i]);
  }
  /* 2. symbol table */
  TraceGlobals((struct LispSymbol *)LispEnv()->symbol_table);
  /* 3. labels */
  rs = read_state;
  while (rs != NULL) {
    items = rs->exprs.items;
    LabelTableRelocate(&rs->exprs);
    for (i = 0; i < rs->exprs.n; i++) {
      rs->exprs.items[i] = LispGcRelocate(items[i]);
    }
    items = rs->labels.items;
    LabelTableRelocate(&rs->labels);
    for (i = 0; i < rs->labels.n; i++) {
      rs->labels.items[i] = LispGcRelocate(items[i]);
    }
    rs = rs->prev;
  }
  /* 4. print_conses */
  items = print_conses.items;
  LabelTableRelocate(&print_conses);
  for (i = 0; i < print_conses.n; i++) {
    print_conses.items[i] = LispGcRelocate(items[i]);
  }
  /* 5. cons_flag */
  cons_flags = LispGcRelocate(cons_flags);

  printf("gc: found %lu live objects occupy %ld/%lu bytes.\n",
         *LispNumberOfObjectsAllocated(),
         (LispIndex)curr_heap - (LispIndex)to_space, HEAP_SIZE);
  temp = to_space;
  to_space = from_space;
  from_space = temp;

  printf("gc2.stack = %ld\n", stack_ptr);

  /* All data was live */
  if ((uintptr_t)curr_heap > (uintptr_t)heap_limit) {
    LispError("objects space overflow.");
  }

  return LISP_T;
}
