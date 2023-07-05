/*
 *    \file objects.c
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
#include "objects.h"

#include "gc.h"
#include "memorylayout.h"
#include "print.h"
#include "utils.h"

static LispIndex gen_sym_ctr = 0;

LispEnvPtr LispEnv() {
  static struct _LispEnv env;
  return &env;
}

#define SAFECAST_OP(l_type, c_type, lisp_to_c_fun)            \
  c_type To##l_type(LispObject v, char *fname) {              \
    if (LISP_##l_type##P(v)) return (c_type)lisp_to_c_fun(v); \
    LispTypeError(fname, #l_type, v);                         \
    return (c_type)0;                                         \
  }
SAFECAST_OP(Cons, struct LispCons *, LISP_CONS_PTR)
SAFECAST_OP(Symbol, struct LispSymbol *, IDENTITY)
SAFECAST_OP(FixNum, LispFixNum, LISP_FIXNUM)
SAFECAST_OP(Character, LispIndex, LISP_CHAR_CODE)
SAFECAST_OP(CFunction, struct LispCFunction *, IDENTITY)
SAFECAST_OP(Vector, struct LispVector *, IDENTITY)
SAFECAST_OP(String, struct LispString *, IDENTITY)
SAFECAST_OP(BitVector, struct LispBitVector *, IDENTITY)
SAFECAST_OP(SingleFloat, struct LispSingleFloat *, IDENTITY)
SAFECAST_OP(DoubleFloat, struct LispDoubleFloat *, IDENTITY)
SAFECAST_OP(LongFloat, struct LispLongFloat *, IDENTITY)
SAFECAST_OP(GenSym, struct LispGenSym *, IDENTITY)

/* numbers */
#define MAKE_FUNC(name, union_t, c_type)            \
  LispObject LispMake##name(c_type val) {           \
    LispObject o_new = LispAllocObject(k##name, 0); \
    o_new->union_t.value = val;                     \
    return o_new;                                   \
  }
MAKE_FUNC(SingleFloat, single_float, float)
MAKE_FUNC(DoubleFloat, double_float, double)
MAKE_FUNC(LongFloat, long_float, long double)

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
  LispIndex n = (LispIndex)strlen(str);
  LispObject obj;
  obj = LispAllocObject(kString, (LispIndex)n - 1);
  obj->string.size = n;
  strncpy(obj->string.self, str, n);
  return obj;
}

/* bit-vector */
#define BIT_VECTOR_SIZE(n) ((LispIndex)((n + 7) >> 3));
LispObject LispBitVectorResize(LispObject bv, LispIndex n) {
  LispObject bv_new;
  LispIndex i = 0, sz = BIT_VECTOR_SIZE(n);
  if (sz > ToBitVector(bv, "bit-vector-resize")->size) {
    bv_new = LispAllocObject(kBitVector, sz - 1);
    bv_new->bit_vector.size = sz;
    for (; i < bv->vector.size; ++i) {
      bv_new->bit_vector.self[i] = bv->bit_vector.self[i];
    }
    for (; i < sz; ++i) {
      bv_new->bit_vector.self[i] = 0;
    }
  } else {
    bv_new = bv;
  }
  return bv_new;
}
LispObject LispMakeInitializedBitVector(LispIndex n, uint8_t val) {
  LispObject bv;
  LispIndex sz = BIT_VECTOR_SIZE(n);
  LispIndex i;
  bv = LispAllocObject(kBitVector, sz - 1);
  bv->bit_vector.size = sz;
  for (i = 0; i < sz; ++i) {
    bv->bit_vector.self[i] = val;
  }
  return bv;
}

LispObject LispMakeBitVector(LispIndex n) {
  LispObject bv;
  LispIndex sz = BIT_VECTOR_SIZE(n);
  bv = LispAllocObject(kBitVector, sz - 1);
  bv->bit_vector.size = sz;
  return bv;
}
LispObject LispMakeBitVectorExactSize(LispIndex n) {
  LispObject bv;
  LispIndex sz = n;
  bv = LispAllocObject(kBitVector, sz - 1);
  bv->bit_vector.size = sz;
  return bv;
}

void LispBitVectorSet(LispObject o, uint32_t n, uint8_t c) {
  uint8_t *b = ToBitVector(o, "bit-vector-set")->self;
  if (c)
    b[n >> 3] |= (uint8_t)(1 << (n & 7));
  else
    b[n >> 3] &= (uint8_t) ~(1 << (n & 7));
}

uint8_t LispBitVectorGet(LispObject o, uint32_t n) {
  uint8_t *b = ToBitVector(o, "bit-vector-get")->self;

  return b[n >> 3] & (uint8_t)(1 << (n & 7));
}
/* vector */
LispObject LispMakeVector(LispIndex size) {
  LispObject vec;
  vec = LispAllocObject(kVector, (LispIndex)size - 1);
  vec->vector.size = size;
  vec->vector.fillp = 0;
  return vec;
}
LispObject LispVectorResize(LispObject v, LispIndex alloc_size) {
  LispObject vec;
  if (alloc_size > ToVector(v, "vector-resize")->size) {
    LispIndex new_size = (alloc_size > v->vector.size * 2u)
                             ? alloc_size
                             : (LispIndex)((alloc_size * 3u) >> 1u);
    vec = LispAllocObject(kVector, new_size - 1);
    vec->vector.size = new_size;
    vec->vector.fillp = v->vector.fillp;
    memcpy(vec->vector.self, v->vector.self,
           sizeof(LispObject) * v->vector.fillp);
  } else {
    vec = v;
  }
  return vec;
}
LispObject LispVectorPush(LispObject v, LispObject value) {
  LispObject vec;
  vec = LispVectorResize(v, (LispIndex)ToVector(v, "vector-push")->fillp + 1);
  vec->vector.self[vec->vector.fillp++] = value;
  return vec;
}
LispObject LispVectorPop(LispObject v) {
  static LispObject vec;
  if (ToVector(v, "vector-pop")->fillp > 0) {
    vec = v->vector.self[v->vector.fillp--];
  } else {
    LispError("pop empty vector.");
  }
  return vec;
}
/* gen-symbol */
LispObject LdMakeGenSym(LispIndex nargs) {
  ArgCount("gensym", nargs, 0);
  LispObject gs = (LispObject)LispAllocObject(kGenSym, 0);
  gs->gen_sym.stype = kSymGenSym;
  gs->gen_sym.id = gen_sym_ctr++;
  return gs;
}

char *LispSymbolName(LispObject sym) {
  char *name;
  if (LISP_SYMBOL_GENSYMP(sym)) {
    name = Uint2Str((char *)scratch_pad + 1, SCRATCH_PAD_SIZE - 1,
                    sym->gen_sym.id, 10);
    *(--name) = 'g';
  } else {
    name = ToSymbol(sym, "symbol-name")->name;
  }
  return name;
}

// conses
// ---------------------------------------------------------------------

LispObject MakeCons(void) { return LispAllocObject(kList, 0); }

LispObject cons_(LispObject car, LispObject cdr) {
  PUSH(cdr);
  PUSH(car);
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = POP();
  LISP_CONS_CDR(c) = POP();
  return c;
}

LispObject cons(LispObject car, LispObject cdr) {
  PUSH(cdr);
  PUSH(car);
  LispObject c = MakeCons();
  LISP_CONS_CAR(c) = POP();
  LISP_CONS_CDR(c) = POP();
  return c;
}
