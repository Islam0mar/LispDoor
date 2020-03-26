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

/* Data allocation */
void *GcMalloc(LispIndex num_of_bytes) {
  void *ptr;
  if (curr_heap + num_of_bytes > heap_limit) {
    LispError("user area overflow.");

    GC();
  }
  if (curr_heap + num_of_bytes > heap_limit) {
    LispError("user area overflow.");
  }
  ptr = (void *)curr_heap;
  curr_heap += num_of_bytes;
  curr_heap =
      (Byte *)(((LispFixNum)curr_heap + (ALIGN_BITS - 1)) & -ALIGN_BITS);
  return ptr;
}

LispObject LispAllocObject(LispType t, LispIndex extra_size) {
  static LispObject obj;
  switch (t) {
    case kList:
      obj = (LispObject)GcMalloc(sizeof(struct LispCons));
      return LISP_PTR_CONS(obj);
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
      obj = (LispObject)SymMalloc(sizeof(struct LispCFunction));
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
LispObject LispCopyObject(LispObject o) {
  LispObject o_new = o;
  LispIndex size = 0;
  if (LISP_UNBOUNDP(o)) {
  } else if (LISP_NULL(o)) {
  } else if (o == LISP_T) {
  } else {
    LispType t = LISP_TYPE_OF(o);
    switch (t) {
      case kSymbol: {
        if (LISP_SYMBOL_GENSYMP(o)) {
          o_new = LispMakeGenSym(0);
          o_new->gen_sym.id = o->gen_sym.id; /* useful for debugging */
        }
      }
      case kFixNum: {
        break;
      }
      case kSingleFloat: {
        o_new = LispMakeSingleFloat(o->single_float.value);
        break;
      }
      case kDoubleFloat: {
        o_new = LispMakeDoubleFloat(o->double_float.value);

        break;
      }
      case kLongFloat: {
        o_new = LispMakeLongFloat(o->long_float.value);
        break;
      }
      case kBitVector: {
        size = o->bit_vector.size;
        o_new = LispMakeBitVector(size);
        memcpy(o_new->bit_vector.self, o->bit_vector.self, size);
        break;
      }
      case kVector: {
        size = o->vector.size;
        o_new = LispMakeVector(size);
        memcpy(o_new->vector.self, o->vector.self, size);
        o_new->vector.fillp = o->vector.fillp;
        break;
      }
      case kString: {
        o_new = LispMakeString(o->string.self);
        break;
      }

      case kList: {
        LispObject a, d, nc, first, *pcdr;
        // iterative implementation allows arbitrarily long cons chains
        pcdr = &first;
        do {
          /* self referring case */
          if ((a = LISP_CONS_CAR(o)) == LISP_UNBOUND) {
            d = LISP_CONS_CDR(o);
            break;
          }
          *pcdr = nc = MakeCons();
          d = LISP_CONS_CDR(o);
          LISP_CONS_CAR(o) = LISP_UNBOUND;
          LISP_CONS_CDR(o) = nc;
          LISP_CONS_CAR(nc) = LispCopyObject(a);
          pcdr = &LISP_CONS_CDR(nc);
          o = d;
        } while (LISP_ConsP(o));
        o_new = d;
      }
      default:
        LispTypeError("object_copy", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
        assert(false);
    }
  }
  return o_new;
}

LispObject GcRelocate(LispObject v) {
  LispObject a, d, nc, first, *pcdr;

  if (!LISP_ConsP(v)) return v;
  if (LISP_CONS_CAR(v) == LISP_UNBOUND) return LISP_CONS_CDR(v);
  nc = MakeCons();
  a = LISP_CONS_CAR(v);
  d = LISP_CONS_CDR(v);
  LISP_CONS_CAR(v) = LISP_UNBOUND;
  LISP_CONS_CDR(v) = nc;
  LISP_CONS_CAR(nc) = GcRelocate(a);
  LISP_CONS_CDR(nc) = GcRelocate(d);
  return nc;
}

void TraceGlobals() {
  struct LispSymbol *root = (struct LispSymbol *)LispEnv()->symbol_table;
  while (root != NULL) {
    root->value = GcRelocate(root->value);
    TraceGlobals(root->left);
    root = root->right;
  }
}
/* TODO */
void GC(void) {
  int grew = 0;
  Byte *temp;
  LispIndex i;
  ReadState *rs;

  curr_heap = to_space;
  uintptr_t lim = curr_heap + HEAP_SIZE - sizeof(union LispUnion);

  for (i = 0; i < stack_ptr; i++) {
    stack[i] = GcRelocate(stack[i]);
  }
  TraceGlobals();
  // #ifdef VERBOSEGC
  printf("gc found %ld/%ld live conses\n", (curr_heap - to_space) / 8,
         HEAP_SIZE / 8);
  // #endif
  temp = to_space;
  to_space = from_space;
  from_space = temp;

  // fixme:
  if ((uintptr_t)curr_heap > lim) {
    LispError("object space overflow.");
  }
}
