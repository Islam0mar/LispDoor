/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#include "objects.h"

#include "gc.h"
#include "memorylayout.h"
#include "print.h"
#include "utils.h"

LispIndex gen_sym_ctr = 0;

const LispEnvPtr LispEnv() {
  static struct _LispEnv env;
  return &env;
}

#define SAFECAST_OP(l_type, c_type, lisp_to_c_fun)            \
  c_type To##l_type(LispObject v, char *fname) {              \
    if (LISP_##l_type##P(v)) return (c_type)lisp_to_c_fun(v); \
    LispTypeError(fname, #l_type, v);                         \
    return (c_type)0;                                         \
  }
SAFECAST_OP(Cons, struct LispCons *, LISP_CONS_PTR);
SAFECAST_OP(Symbol, struct LispSymbol *, IDENTITY);
SAFECAST_OP(FixNum, LispFixNum, LISP_FIXNUM);
SAFECAST_OP(BuiltIn, struct LispSymbol *, LISP_BUILTIN_TO_SYM);
SAFECAST_OP(Vector, struct LispVector *, IDENTITY);
SAFECAST_OP(String, struct LispString *, IDENTITY);
SAFECAST_OP(BitVector, struct LispBitVector *, IDENTITY);
SAFECAST_OP(SingleFloat, struct LispSingleFloat *, IDENTITY);
SAFECAST_OP(DoubleFloat, struct LispDoubleFloat *, IDENTITY);
SAFECAST_OP(LongFloat, struct LispLongFloat *, IDENTITY);
SAFECAST_OP(GenSym, struct LispGenSym *, IDENTITY);

/* string */
LispObject LispMakeString(char *str) {
  LispIndex n = strlen(str);
  LispObject obj;
  obj = LispAllocObject(kString, n);
  strncpy(obj->string.self, str, n);
  return obj;
}

/* bit-vector */
LispObject LispBitVectorResize(LispObject bv, LispIndex n) {
  LispObject bv_new;
  LispIndex i = 0, sz = ((n + 31) >> 5) * sizeof(uint32_t);
  if (LISP_NULL(bv) || sz > bv->bit_vector.size) {
    bv_new = LispAllocObject(kBitVector, sz);
    bv_new->bit_vector.size = sz;
    if (!LISP_NULL(bv)) {
      for (; i < bv->vector.size; ++i) {
        bv_new->bit_vector.self[i] = bv->bit_vector.self[i];
      }
    }
    memset(&bv_new->bit_vector.self[i], 0, sz - i);
  } else {
    bv_new = bv;
  }
  return bv_new;
}

LispObject LispMakeBitVector(LispIndex n) {
  return LispBitVectorResize(LISP_NIL, n);
}

void LispBitVectorSet(LispObject o, uint32_t n, uint32_t c) {
  uint32_t *b = o->bit_vector.self;
  if (c)
    b[n >> 5] |= (1 << (n & 31));
  else
    b[n >> 5] &= ~(1 << (n & 31));
}

uint32_t LispBitVectorGet(LispObject o, uint32_t n) {
  uint32_t *b = o->bit_vector.self;
  return b[n >> 5] & (1 << (n & 31));
}
/* vector */
LispObject LispMakeVector(LispIndex size) {
  LispObject vec;
  vec = LispAllocObject(kVector, size);
  vec->vector.size = size;
  vec->vector.fillp = 0;
  return vec;
}
LispObject LispVectorResize(LispObject v, LispIndex alloc_size) {
  LispObject vec;
  if (alloc_size > v->vector.size) {
    LispIndex i = 0;
    LispIndex new_size = (alloc_size > v->vector.size * 2u)
                             ? alloc_size
                             : ((alloc_size * 3u) >> 1u);
    vec = LispAllocObject(kVector, new_size);
    vec->vector.size = new_size;
    vec->vector.fillp = v->vector.fillp;
    for (i = 0; i < v->vector.size; ++i) {
      vec->vector.self[i] = v->vector.self[i];
    }
  } else {
    vec = v;
  }
  return vec;
}
LispObject LispVectorPush(LispObject v, LispObject value) {
  LispObject vec;
  vec = LispVectorResize(v, v->vector.fillp + 1);
  vec->vector.self[vec->vector.fillp++] = value;
  return vec;
}
LispObject LispVectorPop(LispObject v, LispObject value) {
  LispObject vec;
  if (v->vector.fillp > 0) {
    vec = v->vector.self[vec->vector.fillp--];
  } else {
    LispError("pop empty vector.");
  }
  return vec;
}
/* gen-symbol */
LispObject LispMakeGenSym(LispIndex nargs) {
  argcount("gensym", nargs, 0);
  LispObject gs = (LispObject)LispAllocObject(kGenSym, 0);
  gs->gen_sym.stype = kSymGenSym;
  gs->gen_sym.id = gen_sym_ctr++;
  gs->gen_sym.value.obj = LISP_UNBOUND;
  return gs;
}

char *LispSymbolName(LispObject sym) {
  char *name;
  if (LISP_SYMBOL_GENSYMP(sym)) {
    name = Uint2Str(gs_name + 1, GEN_SYM_NAME_SIZE - 1, sym->gen_sym.id, 10);
    *(--name) = 'g';
  } else {
    name = sym->symbol.name;
  }
  return name;
}

// conses
// ---------------------------------------------------------------------

LispObject MakeCons(void) { return LispAllocObject(kList, 0); }

LispObject cons_(LispObject car, LispObject cdr) {
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = car;
  LISP_CONS_CDR(c) = cdr;
  return c;
}

LispObject cons(LispObject car, LispObject cdr) {
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = car;
  LISP_CONS_CDR(c) = cdr;
  PUSH(c);
  return c;
}
// allocate n consecutive conses
LispObject ConsReserve(LispIndex n) {
  return LISP_PTR_CONS(GcMalloc(n * sizeof(struct LispCons)));
}
