/**
 *   \file objects.c
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

static LispIndex gen_sym_ctr = 0;

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
SAFECAST_OP_HEADER(CFunction, struct LispCFunction *, IDENTITY);
SAFECAST_OP(Vector, struct LispVector *, IDENTITY);
SAFECAST_OP(String, struct LispString *, IDENTITY);
SAFECAST_OP(BitVector, struct LispBitVector *, IDENTITY);
SAFECAST_OP(SingleFloat, struct LispSingleFloat *, IDENTITY);
SAFECAST_OP(DoubleFloat, struct LispDoubleFloat *, IDENTITY);
SAFECAST_OP(LongFloat, struct LispLongFloat *, IDENTITY);
SAFECAST_OP(GenSym, struct LispGenSym *, IDENTITY);

/* numbers */
#define MAKE_FUNC(name, union_t, c_type)            \
  LispObject LispMake##name(c_type val) {           \
    LispObject o_new = LispAllocObject(k##name, 0); \
    o_new->union_t.value = val;                     \
    return o_new;                                   \
  }
MAKE_FUNC(SingleFloat, single_float, float);
MAKE_FUNC(DoubleFloat, double_float, double);
MAKE_FUNC(LongFloat, long_float, long double);

/* c function */
LispObject LispMakeCFunction(char *name, LispFunc fun) {
  LispObject obj = LispAllocObject(kCFunction, 0);
  obj->cfun.f = fun;
  obj->cfun.name = name;
  obj->cfun.f_type = kFunctionOrdinary;
  return obj;
}
LispObject LispMakeCFunctionSpecial(char *name, LispFunc fun) {
  LispObject obj = LispAllocObject(kCFunction, 0);
  obj->cfun.f = fun;
  obj->cfun.name = name;
  obj->cfun.f_type = kFunctionSpecial;
  return obj;
}

/* string */
LispObject LispMakeString(char *str) {
  LispIndex n = strlen(str);
  LispObject obj;
  obj = LispAllocObject(kString, n);
  obj->string.size = n;
  strncpy(obj->string.self, str, n);
  return obj;
}

/* bit-vector */
LispObject LispBitVectorResize(LispObject bv, LispIndex n) {
  LispObject bv_new;
  LispIndex i = 0, sz = ((n + 31) >> 5) * sizeof(uint32_t);
  if (sz > bv->bit_vector.size) {
    bv_new = LispAllocObject(kBitVector, sz);
    bv_new->bit_vector.size = sz;
    for (; i < bv->vector.size; ++i) {
      bv_new->bit_vector.self[i] = bv->bit_vector.self[i];
    }
    memset(&bv_new->bit_vector.self[i], 0, sz - i);
  } else {
    bv_new = bv;
  }
  return bv_new;
}
LispObject LispMakeInitializedBitVector(LispIndex n, int val) {
  LispObject bv;
  LispIndex sz = ((n + 31) >> 5) * sizeof(uint32_t);
  bv = LispAllocObject(kBitVector, sz);
  bv->bit_vector.size = sz;
  memset(bv->bit_vector.self, val, sz);
  return bv;
}

LispObject LispMakeBitVector(LispIndex n) {
  LispObject bv;
  LispIndex sz = ((n + 31) >> 5) * sizeof(uint32_t);
  bv = LispAllocObject(kBitVector, sz);
  bv->bit_vector.size = sz;
  return bv;
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
  static LispObject vec;
  if (v->vector.fillp > 0) {
    vec = v->vector.self[v->vector.fillp--];
  } else {
    LispError("pop empty vector.");
  }
  return vec;
}
/* gen-symbol */
LispObject LispMakeGenSym(LispIndex nargs) {
  ArgCount("gensym", nargs, 0);
  LispObject gs = (LispObject)LispAllocObject(kGenSym, 0);
  gs->gen_sym.stype = kSymGenSym;
  gs->gen_sym.id = gen_sym_ctr++;
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
