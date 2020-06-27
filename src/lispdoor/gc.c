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
#include "lispdoor/gc.h"

#include "lispdoor/memorylayout.h"
#include "lispdoor/print.h"
#include "lispdoor/symboltree.h"
#include "lispdoor/utils.h"

#define OBJ_INDEX(c)                                            \
  ((LispSmallestStruct *)((LispFixNum)c & ~((LispFixNum)0x3)) - \
   (LispSmallestStruct *)((LispFixNum)heap))

#define CONS_P(c) LispBitVectorGet(gc_cons, (uint32_t)OBJ_INDEX(c))
#define SET_CONS(c) LispBitVectorSet(gc_cons, (uint32_t)OBJ_INDEX(c), 1)
#define UNSET_CONS(c) LispBitVectorSet(gc_cons, (uint32_t)OBJ_INDEX(c), 0)
#define MARKED_P(c) LispBitVectorGet(gc_mark_bit, (uint32_t)OBJ_INDEX(c))
#define MARK_OBJ(c) LispBitVectorSet(gc_mark_bit, (uint32_t)OBJ_INDEX(c), 1)
#define UNMARK_OBJ(c) LispBitVectorSet(gc_mark_bit, (uint32_t)OBJ_INDEX(c), 0)

LispIndex *LispNumberOfObjectsAllocated() {
  static LispIndex objects = 0;
  return &objects;
}

/* Data allocation */
void *GcMalloc(LispIndex num_of_bytes) {
  void *ptr;
  if ((LispFixNum)curr_heap + num_of_bytes > (LispFixNum)heap + HEAP_SIZE) {
    GC(0);
  }
  if ((LispFixNum)curr_heap + num_of_bytes > (LispFixNum)heap + HEAP_SIZE) {
    LispError("no space to allocate new object.");
  }
  ptr = (void *)curr_heap;
  curr_heap = (Byte *)((LispFixNum)curr_heap + num_of_bytes);
  curr_heap =
      (Byte *)(((LispFixNum)curr_heap + (ALIGN_BITS - 1)) & -ALIGN_BITS);
  ++*LispNumberOfObjectsAllocated(); /* better readability */
  return ptr;
}
LispObject GcNextHeapObject(LispObject obj) {
  LispIndex l = 0;
  if (CONS_P(obj)) {
    obj = LISP_PTR_CONS(obj);
  }
  LispType t = LISP_TYPE_OF(obj);
  switch (t) {
    case kList:
      obj = (LispObject)LISP_CONS_PTR(obj);
      l = sizeof(struct LispCons);
      break;
    case kSingleFloat:
      l = sizeof(struct LispSingleFloat);
      break;
    case kDoubleFloat:
      l = sizeof(struct LispDoubleFloat);
      break;
    case kLongFloat:
      l = sizeof(struct LispLongFloat);
      break;
    case kSymbol:
      if (LISP_GenSymP(obj)) {
        l = sizeof(struct LispGenSym);
      } else {
        l = sizeof(struct LispSymbol) + strlen(obj->symbol.name) * sizeof(char);
      }
      break;
    case kCFunction:
      l = sizeof(struct LispCFunction);
      break;
    case kBitVector:
      l = sizeof(struct LispBitVector) +
          (sizeof(uint8_t) * (obj->bit_vector.size - 1));
      break;
    case kString:
      l = sizeof(struct LispString) + (obj->string.size - 1) * sizeof(char);
      break;
    case kVector:
      l = sizeof(struct LispVector) +
          (sizeof(LispObject) * (obj->vector.size - 1));
      break;
    default:
      LispError("error: Unkown object located at the heap!\n");
      break;
  }

  obj = (LispObject)((LispFixNum)obj + l);
  obj = (LispObject)(((LispFixNum)obj + (ALIGN_BITS - 1)) & -ALIGN_BITS);
  if (CONS_P(obj)) {
    obj = LISP_PTR_CONS(obj);
  }
  return obj;
}

void GcMarkObject(LispObject o) {
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
      case kSymbol: {
        MARK_OBJ(o);
        if (!LISP_SYMBOL_GENSYMP(o)) {
          GcMarkObject(o->symbol.value);
        }
        break;
      }
      case kSingleFloat:
      case kDoubleFloat:
      case kLongFloat:
      case kCFunction:
      case kBitVector:
      case kString: {
        MARK_OBJ(o);
        break;
      }
      case kVector: {
        MARK_OBJ(o);
        LispIndex i = 0;
        for (i = 0; i < o->vector.fillp; ++i) {
          GcMarkObject((o->vector.self[i]));
        }
        break;
      }
      case kList: {
        LispObject a, d;
        do {
          MARK_OBJ(o);
          gc_offset->vector.self[OBJ_INDEX(o)] = o;
          a = LISP_CONS_CAR(o);
          d = LISP_CONS_CDR(o);
          GcMarkObject(a);
          o = d;
        } while (LISP_ConsP(o));
        GcMarkObject(o);
        break;
      }
      default:
        LispTypeError("GcMarkObject", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
        break;
    }
  }
}

void GcMarkLiveObjects() {
  LispIndex i;
  ReadState *rs;
  /* 1. stack values */
  for (i = 0; i < (LispIndex)stack_index; i++) {
    GcMarkObject(stack[i]);
  }
  /* 2. symbols */
  GcMarkObject(LispEnv()->symbols);
  /* 3. labels */
  rs = read_state;
  while (rs != NULL) {
    GcMarkObject(rs->exprs.items);
    GcMarkObject(rs->labels.items);
    rs = rs->prev;
  }
  /* 4. print_conses */
  GcMarkObject(print_conses.items);

  /* 5. cons_flag */
  GcMarkObject(cons_flags);
  GcMarkObject(gc_cons);
  GcMarkObject(gc_offset);
  GcMarkObject(gc_mark_bit);
}

LispObject LispAllocObject(LispType t, LispIndex extra_size) {
  static LispObject obj = LISP_NIL;
  switch (t) {
    case kList:
      obj = (LispObject)GcMalloc(sizeof(struct LispCons));
      SET_CONS(obj);
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
                                 extra_size * sizeof(char));
      break;
    case kCFunction:
      obj = (LispObject)GcMalloc(sizeof(struct LispCFunction));
      break;
    case kBitVector:
      obj = (LispObject)GcMalloc(sizeof(struct LispBitVector) +
                                 extra_size * sizeof(uint8_t));
      break;
    case kString:
      obj = (LispObject)GcMalloc(sizeof(struct LispString) +
                                 extra_size * sizeof(char));
      break;
    case kGenSym:
      t = kSymbol;
      obj = (LispObject)GcMalloc(sizeof(struct LispGenSym));
      break;
    case kVector:
      obj = (LispObject)GcMalloc(sizeof(struct LispVector) +
                                 sizeof(LispObject) * extra_size);
      break;
    default:
      LispError("error: wrong object type, alloc botch.\n");
  }
  obj->d.t = (uint8_t)t;
  return obj;
}

// collector
// ------------------------------------------------------------------
void GcComputeLocations() {
  LispIndex offset = 0;
  LispObject curr = (LispObject)heap, prev;
  while ((uintptr_t)curr + 1 < (uintptr_t)curr_heap) {
    if (MARKED_P(curr)) {
      gc_offset->vector.self[OBJ_INDEX(curr)] = LISP_MAKE_FIXNUM(offset);
      curr = GcNextHeapObject(curr);
    } else {
      (*LispNumberOfObjectsAllocated())--;
      prev = curr;
      curr = GcNextHeapObject(curr);
      /* Avoid cons immediate type bit */
      offset += ((LispFixNum)curr & ~0x3) - ((LispFixNum)prev & ~0x3);
    }
  }
  heap_free = (Byte *)((LispFixNum)curr_heap - offset);
}

LispObject GcForwardChildObject(LispObject o) {
  LispObject o_new = o;
  if (LISP_UNBOUNDP(o)) {
  } else if (LISP_NULL(o)) {
  } else if (o == LISP_T) {
  } else if ((((LispFixNum)o & 3) < 2) && !MARKED_P(o)) {
    /* avoid checking char and fixnum */
    o_new = gc_offset->vector.self[OBJ_INDEX(o)];
  } else {
    LispType t = LISP_TYPE_OF(o);
    switch (t) {
      case kCharacter:
      case kFixNum: {
        break;
      }
      case kSymbol:
      case kSingleFloat:
      case kDoubleFloat:
      case kLongFloat:
      case kCFunction:
      case kBitVector:
      case kVector:
      case kString: {
        o_new = (LispObject)((LispFixNum)o -
                             LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o)]));
        break;
      }
      case kList: {
        o_new =
            LISP_PTR_CONS((LispFixNum)LISP_CONS_PTR(o) -
                          LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o)]));
        break;
      }
      default:
        LispTypeError("GcForwardChildObject", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
        break;
    }
  }
  return o_new;
}
LispObject GcForwardObject(LispObject o) {
  LispObject o_new = o;
  if (LISP_UNBOUNDP(o)) {
  } else if (LISP_NULL(o)) {
  } else if (o == LISP_T) {
  } else if ((((LispFixNum)o & 3) < 2) && !MARKED_P(o)) {
    /* avoid checking char and fixnum */
    o_new = gc_offset->vector.self[OBJ_INDEX(o)];
  } else {
    LispType t = LISP_TYPE_OF(o);
    switch (t) {
      case kCharacter:
      case kFixNum: {
        break;
      }
      case kSymbol: {
        if (!LISP_SYMBOL_GENSYMP(o)) {
          o->symbol.value = GcForwardChildObject(o->symbol.value);
        }
        o_new = (LispObject)((LispFixNum)o -
                             LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o)]));
        break;
      }
      case kSingleFloat:
      case kDoubleFloat:
      case kLongFloat:
      case kCFunction:
      case kBitVector:
      case kString: {
        o_new =
            (LispObject)((LispFixNum)o_new -
                         LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o_new)]));
        break;
      }
      case kVector: {
        LispIndex i = 0;
        for (i = 0; i < o->vector.fillp; ++i) {
          o->vector.self[i] = GcForwardChildObject(o->vector.self[i]);
        }
        o_new =
            (LispObject)((LispFixNum)o_new -
                         LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o_new)]));
        break;
      }
      case kList: {
        LISP_CONS_CAR(o) = GcForwardChildObject(LISP_CONS_CAR(o));
        LISP_CONS_CDR(o) = GcForwardChildObject(LISP_CONS_CDR(o));
        o_new =
            LISP_PTR_CONS((LispFixNum)LISP_CONS_PTR(o) -
                          LISP_FIXNUM(gc_offset->vector.self[OBJ_INDEX(o)]));
        break;
      }
      default:
        LispTypeError("GcForwardObject", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
        break;
    }
    UNMARK_OBJ(o);
    gc_offset->vector.self[OBJ_INDEX(o)] = o_new;
  }
  return o_new;
}

void GcUpdateObjectsRelocate() {
  LispIndex i = 0;
  LispObject curr = (LispObject)heap, next;
  ReadState *rs;
  while ((uintptr_t)curr + 1 < (uintptr_t)curr_heap) {
    next = GcNextHeapObject(curr);
    if (MARKED_P(curr)) {
      LispObject new_loc = GcForwardObject(curr);
      if (CONS_P(curr)) {
        UNSET_CONS(curr);
        SET_CONS(new_loc);
        new_loc = (LispObject)LISP_CONS_PTR(new_loc);
        curr = (LispObject)LISP_CONS_PTR(curr);
      }
      memmove(new_loc, curr,
              (size_t)(((LispFixNum)next & ~0x3) - (LispFixNum)curr));
    } else {
      if (CONS_P(curr)) {
        UNSET_CONS(curr);
      }
    }
    curr = next;
  }
  /* 1. stack values */
  for (i = 0; i < (LispIndex)stack_index; i++) {
    stack[i] = GcForwardChildObject(stack[i]);
  }
  /* 2. symbols */
  LispEnv()->symbols = GcForwardChildObject(LispEnv()->symbols);
  /* 3. labels */
  rs = read_state;
  while (rs != NULL) {
    rs->exprs.items = GcForwardChildObject(rs->exprs.items);
    rs->labels.items = GcForwardChildObject(rs->labels.items);
    rs = rs->prev;
  }
  /* 4. print_conses */
  print_conses.items = GcForwardChildObject(print_conses.items);

  /* 5. cons_flag */
  cons_flags = GcForwardChildObject(cons_flags);
  /* GcMarkObject(gc_cons); */
  /* GcMarkObject(gc_offset); */
  /* GcMarkObject(gc_mark_bit); */
}

void GcCompact() {
  GcComputeLocations();
  GcUpdateObjectsRelocate();
}

void GC() {
  memset(gc_mark_bit->bit_vector.self, 0, gc_mark_bit->bit_vector.size);
  memset(gc_offset->vector.self, 0,
         sizeof(LispObject) * gc_offset->vector.size);
  GcMarkLiveObjects();
  GcCompact();
  curr_heap = heap_free;

  LispPrintStr("gc: found ");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        *LispNumberOfObjectsAllocated(), 10));
  LispPrintStr(" live objects occupy ");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        (uint32_t)((LispFixNum)curr_heap - (LispFixNum)heap),
                        10));
  LispPrintStr("/");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, HEAP_SIZE, 10));
  LispPrintStr(" bytes.\n");

  /* All data was live */
  if ((LispFixNum)curr_heap >= (LispFixNum)heap + HEAP_SIZE) {
    LispError("objects space overflow.");
  }
}
