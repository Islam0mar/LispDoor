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
    /* case kCharacter: */
    /*   return LISP_CODE_CHAR(' '); /\* Immediate character *\/ */
    case kLongFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispLongFloat));
      break;
    case kSingleFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispSingleFloat));
      break;
    case kDoubleFloat:
      obj = (LispObject)GcMalloc(sizeof(struct LispDoubleFloat));
      break;
    case kSymbol:
      obj = (LispObject)SymMalloc(sizeof(struct LispSymbol) +
                                  extra_size * sizeof(char) / sizeof(Byte));
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

LispObject GcRelocate(LispObject v) {
  LispObject a, d, nc;

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
    if (!(LISP_STRUCT_BUILTINP(root))) {
      root->value.obj = GcRelocate(root->value.obj);
    }
    TraceGlobals(root->left);
    root = root->right;
  }
}
/* TODO */
void GC(void) {
  int grew = 0;
  unsigned char *temp;
  uint32_t i;

  curr_heap = to_space;
  unsigned char *lim = curr_heap + HEAP_SIZE - sizeof(struct LispCons);

  for (i = 0; i < stack_ptr; i++) stack[i] = GcRelocate(stack[i]);
  TraceGlobals();
  // #ifdef VERBOSEGC
  printf("gc found %ld/%ld live conses\n", (curr_heap - to_space) / 8,
         HEAP_SIZE / 8);
  // #endif
  temp = to_space;
  to_space = from_space;
  from_space = temp;

  // fixme:
  if (curr_heap > lim) {
    LispError("object space overflow.");
  }
}
