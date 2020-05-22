/*
 *    \file gc.c
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
 *    modification, are permitted provided that the following conditions are met:
 *
 *        * Redistributions of source code must retain the above copyright notice,
 *          this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright notice,
 *          this list of conditions and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution.
 *        * Neither the author nor the names of any contributors may be used to
 *          endorse or promote products derived from this software without specific
 *          prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "lispdoor/gc.h"

#include "lispdoor/memorylayout.h"
#include "lispdoor/print.h"
#include "lispdoor/symboltree.h"
#include "lispdoor/utils.h"
#include "stm32f1xx_hal.h"

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
  static LispObject obj = LISP_NIL;
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
      obj = (LispObject)GcMalloc(sizeof(struct LispSymbol) +
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
      LispError("error: wrong object type, alloc botch.\n");
  }
  obj->d.t = (uint8_t)t;
  return obj;
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
        } else {
          o_new = LispAllocObject(kSymbol, (LispIndex)strlen(o->symbol.name));
          o_new->symbol.stype = o->symbol.stype;
          strcpy(o_new->symbol.name, o->symbol.name);
          o_new->symbol.height = o->symbol.height;
          /* dfs */
          LispObject left = (LispObject)o->symbol.left,
                     right = (LispObject)o->symbol.right,
                     value = o->symbol.value;

          LISP_SET_FORWARDED(o, o_new);
          o_new->symbol.value = LispGcRelocate(value);
          o_new->symbol.left = (struct LispSymbol *)LispGcRelocate(left);
          o_new->symbol.right = (struct LispSymbol *)LispGcRelocate(right);
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
        LISP_SET_FORWARDED(o, o_new);
        LispObject *self = o->vector.self;
        LispIndex i = 0;
        for (i = 0; i < size; ++i) {
          o_new->vector.self[i] = LispGcRelocate(self[i]);
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
    }
  }
  return o_new;
}

void LispGcTraceGlobals() {
  LispEnv()->symbol_tree = LispGcRelocate(LispEnv()->symbol_tree);
}

void GC() {
  Byte *temp;
  LispIndex i;
  ReadState *rs;
  LispObject *items;

  curr_heap = to_space;
  heap_limit = curr_heap + HEAP_SIZE;
  /* trace number of objects allocated */
  *LispNumberOfObjectsAllocated() = 0;
  /* 1. stack values */
  for (i = 0; i < (LispIndex)stack_index; i++) {
    stack[i] = LispGcRelocate(stack[i]);
  }
  /* 2. symbol tree */
  LispGcTraceGlobals();
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

  LispPrintStr("gc: found ");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        *LispNumberOfObjectsAllocated(), 10));
  LispPrintStr(" live objects occupy ");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        (uintptr_t)curr_heap - (uintptr_t)to_space, 10));
  LispPrintStr("/");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, HEAP_SIZE, 10));
  LispPrintStr(" bytes.\n");

  temp = to_space;
  to_space = from_space;
  from_space = temp;

  /* All data was live */
  if ((uintptr_t)curr_heap > (uintptr_t)heap_limit) {
    LispError("objects space overflow.");
  }
}
