/*
 *    \file print.c
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

#include "lispdoor/print.h"

#include "hal/bsp.h"
#include "lispdoor/memorylayout.h"
#include "lispdoor/read.h"
#include "lispdoor/utils.h"

#define CONS_INDEX(c)                         \
  (((LispSmallestStruct *)LISP_CONS_PTR(c)) - \
   ((LispSmallestStruct *)from_space))

#define MARKED_P(c) LispBitVectorGet(cons_flags, (uint32_t)CONS_INDEX(c))
#define MARK_CONS(c) LispBitVectorSet(cons_flags, (uint32_t)CONS_INDEX(c), 1)
#define UNMARK_CONS(c) LispBitVectorSet(cons_flags, (uint32_t)CONS_INDEX(c), 0)

// error utilities ------------------------------------------------------------
void LispError(char *format) {
  read_state = NULL;
  LispPrintStr(format);
  longjmp(LispEnv()->top_level, 1);
}
void LispTypeError(char *fname, char *expected, LispObject got) {
  LispPrintStr(fname);
  LispPrintStr(": error: expected ");
  LispPrintStr(expected);
  LispPrintStr(", got ");
  LispPrintObject(got, false);
  LispError("\n");
}

static void PrintTraverse(LispObject v) {
  while (LISP_ConsP(v) && !LISP_UNBOUNDP(LISP_CONS_CAR(v))) {
    if (MARKED_P(v)) {
      LabelTableAdjoin(&print_conses, v);
      break;
    }
    MARK_CONS(v);
    PUSH(v);
    PrintTraverse(LISP_CONS_CAR(v));
    v = POP();
    v = LISP_CONS_CDR(v);
  }
}

static void PrintSymbol(char *name) {
  int i, escape = 0, charescape = 0;

  if (name[0] == '\0') {
    LispPrintStr("||");
    return;
  }
  if (name[0] == '.' && name[1] == '\0') {
    LispPrintStr("|.|");
    return;
  }
  if (name[0] == '#') escape = 1;
  i = 0;
  while (name[i]) {
    if (!SymCharP(name[i])) {
      escape = 1;
      if (name[i] == '|' || name[i] == '\\') {
        charescape = 1;
        break;
      }
    }
    i++;
  }
  if (escape) {
    if (charescape) {
      LispPrintByte('|');
      i = 0;
      while (name[i]) {
        if (name[i] == '|' || name[i] == '\\') {
          LispPrintByte('\\');
          LispPrintByte(name[i]);
        } else {
          LispPrintByte(name[i]);
        }
        i++;
      }
      LispPrintByte('|');
    } else {
      LispPrintByte('|');
      LispPrintStr(name);
      LispPrintByte('|');
    }
  } else {
    LispPrintStr(name);
  }
}

static void DoPrint(LispObject o, bool princ) {
  LispObject cd;
  LispIndex label;
  if (LISP_UNBOUNDP(o)) {
    LispPrintStr("unbound");
  } else if (LISP_NULL(o)) {
    LispPrintStr("nil");
  } else if (o == LISP_T) {
    LispPrintStr("t");
  } else {
    switch (LISP_TYPE_OF(o)) {
      case kFixNum: {
        LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                              LISP_FIXNUM(o), lisp_number_base));
        break;
      }
      case kCharacter: {
        LispPrintStr("#\\");
        LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                              LISP_CHAR_CODE(o), lisp_number_base));
        break;
      }
      case kSingleFloat: {
        LispPrintStr(Float2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                               LISP_SINGLE_FLOAT(o),
                               lisp_fractional_precision));
        break;
      }
      case kDoubleFloat: {
        LispPrintStr(Double2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                                LISP_DOUBLE_FLOAT(o),
                                lisp_fractional_precision));
        break;
      }
      case kString: {
        LispPrintByte('\"');
        LispPrintStrN(o->string.self, o->string.size);
        LispPrintByte('\"');
        break;
      }
      case kBitVector: {
        LispPrintStr("#b(");
        for (label = 0;;) {
          LispPrintStr("0b");
          LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                                o->bit_vector.self[label], 2));
          if (++label < o->bit_vector.size) {
            LispPrintByte(' ');
          } else {
            break;
          }
        }
        LispPrintByte(')');
        break;
      }
      case kForwarded: {
        LispPrintStr("{FORWARDED: ");
        DoPrint(LISP_FORWARD(o), princ);
        LispPrintByte('}');
        break;
      }
      case kCFunction: {
        LispPrintStr("#.");
        if (princ) {
          LispPrintStr(o->cfun.name);
        } else {
          PrintSymbol(o->cfun.name);
        }
        break;
      }
      case kSymbol: {
        if (princ) {
          LispPrintStr(LispSymbolName(o));
        } else {
          PrintSymbol(LispSymbolName(o));
        }
        break;
      }
      case kList: {
        label = LabelTableLookUp(&print_conses, o);
        if (label != NOTFOUND) {
          if (!MARKED_P(o)) {
            LispPrintByte('#');
            LispPrintStr(
                Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, label, 10));
            LispPrintByte('#');
            break;
          }
          LispPrintByte('#');
          LispPrintStr(
              Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, label, 10));
          LispPrintByte('=');
        }
        LispPrintByte('(');
        while (1) {
          UNMARK_CONS(o);
          DoPrint(LISP_CONS_CAR(o), princ);
          cd = LISP_CONS_CDR(o);
          if (!LISP_ConsP(cd)) {
            if (cd != LISP_NIL) {
              LispPrintStr(" . ");
              DoPrint(cd, princ);
            }
            LispPrintByte(')');
            break;
          } else {
            if (LabelTableLookUp(&print_conses, cd) != NOTFOUND) {
              LispPrintStr(" . ");
              DoPrint(cd, princ);
              LispPrintByte(')');
              break;
            }
          }
          LispPrintByte(' ');
          o = cd;
        }
        break;
      }
      default:
        LispTypeError("print", "type within known range",
                      LISP_MAKE_FIXNUM(LISP_TYPE_OF(o)));
    }
  }
}

void LispPrintObject(LispObject v, bool princ) {
  LabelTableClear(&print_conses);
  PUSH(v);
  PrintTraverse(v);
  v = POP();
  DoPrint(v, princ);
}
void LispPrintStr(char *str) { UART1_SendStr(str); }
void LispPrintByte(Byte c) { UART1_SendByte(c); }
void LispPrintStrN(char *s, LispIndex len) { UART1_SendStrN(s, len); }
