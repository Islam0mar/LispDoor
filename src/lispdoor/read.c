/*
 *    \file read.c
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

#include "lispdoor/read.h"

#include "lispdoor/eval.h"
#include "lispdoor/memorylayout.h"
#include "lispdoor/print.h"
#include "lispdoor/read.h"
#include "lispdoor/symboltree.h"
#include "lispdoor/utils.h"

typedef enum {
  kTokNone,
  kTokOpen,
  kTokClose,
  kTokDot,
  kTokQuote,
  kTokSym,
  kTokNum,
  kTokSingleFloat,
  kTokDoubleFloat,
  kTokString,
  kTokVector,
  kTokBitVector,
  kTokBackQuote,
  kTokComma,
  kTokCommaAt,
  kTokCommaDot,
  kTokSharpDot,
  kTokLabel,
  kTokBackRef,
  kTokSharpQuote,
  kTokSharpOpen,
  kTokOpenB,
  kTokCloseB,
  kTokSharpSym,
  kTokGenSym,
  kTokDoubleQuote
} LispTokenType;

LispTokenType toktype = kTokNone;
LispObject tokval;

static const uint32_t offsets_from_utf8[6] = {0x00000000UL, 0x00003080UL,
                                              0x000E2080UL, 0x03C82080UL,
                                              0xFA082080UL, 0x82082080UL};

static const uint8_t trailing_bytes_for_utf8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};

bool SymCharP(char c) {
  static char *special = "()';`,\\|";
  return (!isspace(c) && !strchr(special, c));
}
static inline bool TibEmpty() {
  return (terminal_buffer_get_index == terminal_buffer_insert_index);
}
uint8_t TibReadChar() {
  while (TibEmpty()) {
    /* TODO: multitasking */
    /* pause(); */
  }
  uint8_t index = terminal_buffer_get_index;
  ++terminal_buffer_get_index;
  terminal_buffer_get_index &= (TIB_SIZE - 1);
  return terminal_buffer[index];
}
void TibUnReadChar() {
  --terminal_buffer_get_index;
  terminal_buffer_get_index &= (TIB_SIZE - 1);
}
void TibFlush() { terminal_buffer_get_index = terminal_buffer_insert_index; }

/* get char */
static inline uint8_t GetChar() { return TibReadChar(); }
static inline void UnGetChar() { TibUnReadChar(); }

uint8_t UTF8SeqLen(const uint8_t c) { return trailing_bytes_for_utf8[c] + 1; }

uint32_t UTF8GetChar() {
  uint8_t amt = 0, sz, c;
  uint32_t ch = 0;

  c = GetChar();
  if (c == EOF) return UEOF;
  ch = (uint32_t)c;
  amt = sz = UTF8SeqLen(c);
  while (--amt) {
    ch <<= 6;
    c = GetChar();
    if (c == EOF) return UEOF;
    ch += (uint32_t)c;
  }
  ch -= offsets_from_utf8[sz - 1];

  return ch;
}

char nextchar() {
  char c;
  uint8_t ch;

  do {
    ch = GetChar();
    if (ch == EOF) return 0;
    c = (char)ch;
    if (c == ';') {
      // single-line comment
      do {
        ch = GetChar();
        if (ch == EOF) return 0;
      } while ((char)ch != '\n');
      c = (char)ch;
    }
  } while (isspace(c));
  return c;
}

void take(void) { toktype = kTokNone; }

void accumchar(char c, LispIndex *pi) {
  scratch_pad[(*pi)++] = (uint8_t)c;
  if (*pi >= (LispIndex)(SCRATCH_PAD_SIZE - 1)) {
    LispError("read: error: token too long\n");
  }
}

/* return: true for dot token, false for symbol */
bool read_token(char c, bool digits) {
  LispIndex i = 0, escaped = 0, dot = (c == '.'), totread = 0;
  uint8_t ch;

  UnGetChar();
  while (1) {
    ch = GetChar();
    totread++;
    if (ch == EOF) goto terminate;
    c = (char)ch;
    if (c == '|') {
      escaped = !escaped;
    } else if (c == '\\') {
      ch = GetChar();
      if (ch == EOF) goto terminate;
      accumchar((char)ch, &i);
    } else if (!escaped && !(SymCharP(c) && (!digits || isdigit(c)))) {
      break;
    } else {
      accumchar(c, &i);
    }
  }
  UnGetChar();
terminate:
  scratch_pad[i++] = '\0';
  return (dot && (totread == 2));
}

LispTokenType peek() {
  uint8_t c, *end;
  LispFixNum x;

  if (toktype != kTokNone) return toktype;
  c = nextchar();
  if (c == EOF) return kTokNone;
  if (c == '(') {
    toktype = kTokOpen;
  } else if (c == ')') {
    toktype = kTokClose;
  } else if (c == '\'') {
    toktype = kTokQuote;
  } else if (c == '`') {
    toktype = kTokBackQuote;
  } else if (c == '#') {
    c = GetChar();
    if (c == EOF) LispError("read: error: invalid read macro\n");
    if ((char)c == '.') {
      toktype = kTokSharpDot;
    } else if ((char)c == '\'') {
      toktype = kTokSharpQuote;
    } else if ((char)c == '\\') {
      /* FixMe: */
      toktype = kTokNum;
      tokval = LISP_MAKE_CHARACTER(UTF8GetChar());
    } else if (isdigit(c)) {
      read_token((char)c, true);
      c = GetChar();
      if (c == '#')
        toktype = kTokBackRef;
      else if (c == '=')
        toktype = kTokLabel;
      else
        LispError("read: error: invalid label\n");
      x = strtol((char *)scratch_pad, (char **)&end, 10);
      if (*end != '\0') {
        LispError("read: error: invalid label\n");
      }
      tokval = LISP_MAKE_FIXNUM(x);
    } else {
      LispError("read: error: unknown read macro\n");
    }
  } else if (c == ',') {
    toktype = kTokComma;
    c = GetChar();
    if (c == EOF) return toktype;
    if ((char)c == '@')
      toktype = kTokCommaAt;
    else if ((char)c == '.')
      toktype = kTokCommaDot;
    else
      UnGetChar();
  } else if (isdigit(c) || c == '-' || c == '+') {
    read_token(c, false);
    if (c == '0' && (scratch_pad[1] == 'b')) {
      x = strtol((char *)scratch_pad + 2, (char **)&end, 2);
    } else {
      x = strtol((char *)scratch_pad, (char **)&end, 0);
    }
    if (*end != '\0') {
      toktype = kTokSym;
      tokval = LispMakeSymbol((char *)scratch_pad);
    } else {
      toktype = kTokNum;
      tokval = LISP_MAKE_FIXNUM(x);
    }
  } else {
    if (read_token(c, false)) {
      toktype = kTokDot;
    } else {
      toktype = kTokSym;
      tokval = LispMakeSymbol((char *)scratch_pad);
    }
  }
  return toktype;
}

/* Parser                   tokens --> ast */
LispObject do_read_sexpr(LispIndex fixup);
/* build a list of conses. this is complicated by the fact that all conses
 * can move whenever a new cons is allocated. we have to refer to every cons
 * through a handle to a relocatable pointer (i.e. a pointer on the stack). */
void read_list(LispObject *pval, LispIndex fixup) {
  LispObject c, *pc;
  uint32_t t;

  PUSH(LISP_NIL);
  pc = &stack[stack_index - 1];  // to keep track of current cons cell
  t = peek();
  while (t != kTokClose) {
    if (t == EOF) {
      LispError("read: error: unexpected end of input\n");
    }
    c = MakeCons();
    LISP_CONS_CAR(c) = LISP_CONS_CDR(c) = LISP_NIL;
    if (LISP_ConsP(*pc)) {
      LISP_CONS_CDR(*pc) = c;
    } else {
      *pval = c;
      if (fixup != NOTFOUND) {
        read_state->exprs.items->vector.self[fixup] = c;
      }
    }
    *pc = c;
    c = do_read_sexpr(NOTFOUND);  // must be on separate lines due to undefined
    LISP_CONS_CAR(*pc) = c;       // evaluation order

    t = peek();
    if (t == kTokDot) {
      take();
      c = do_read_sexpr(NOTFOUND);
      LISP_CONS_CDR(*pc) = c;
      t = peek();
      if (t == EOF) {
        LispError("read: error: unexpected end of input\n");
      }
      if (t != kTokClose) {
        LispError("read: error: expected ')'\n");
      }
    }
  }
  take();
  POPN(1);
}

/* fixup is the index of the label we'd like to fix up with this read */
LispObject do_read_sexpr(LispIndex fixup) {
  LispObject v = LISP_NIL, head;
  LispIndex i;
  uint32_t t = peek();
  bool list_p = false;
  take();
  switch (t) {
    case kTokClose: {
      LispError("read: error: unexpected ')'\n");
      break;
    }
    case kTokDot: {
      LispError("read: error: unexpected '.'\n");
      break;
    }
    case kTokSym:
    case kTokNum: {
      v = tokval;
      break;
    }
    case kTokComma: {
      head = LispMakeSymbol("*comma*");
      list_p = true;
      break;
    }
    case kTokCommaAt: {
      head = LispMakeSymbol("*comma-at*");
      list_p = true;
      break;
    }
    case kTokCommaDot: {
      head = LispMakeSymbol("*comma-dot*");
      list_p = true;
      break;
    }
    case kTokBackQuote: {
      head = LispMakeSymbol("backquote");
      list_p = true;
      break;
    }
    case kTokQuote: {
      head = LispMakeSymbol("quote");
      list_p = true;
      break;
    }
    case kTokSharpQuote: {
      v = do_read_sexpr(fixup);
      break;
    }
    case kTokOpen: {
      PUSH(LISP_NIL);
      read_list(&stack[stack_index - 1], fixup);
      v = POP();
      break;
    }
    case kTokSharpDot: {
      /* eval-when-read */
      /*   evaluated expressions can refer to existing backreferences, but they
       */
      /*   cannot see pending labels. in other words: */
      /*   (... #2=#.#0# ... )    OK */
      /*   (... #2=#.(#2#) ... )  DO NOT WANT */
      v = do_read_sexpr(NOTFOUND);
      v = TopLevelEval(v);
      break;
    }
    case kTokLabel: {
      /* create backreference label */
      if (LabelTableLookUp(&read_state->labels, tokval) != NOTFOUND) {
        LispPrintStr("read: error: label ");
        LispPrintObject(tokval, false);
        LispError(" redefined\n");
      }
      LabelTableInsert(&read_state->labels, tokval);
      i = read_state->exprs.items->vector.fillp;
      LabelTableInsert(&read_state->exprs, LISP_UNBOUND);
      v = do_read_sexpr(i);
      read_state->exprs.items->vector.self[i] = v;
      break;
    }
    case kTokBackRef: {
      /* look up backreference */
      i = LabelTableLookUp(&read_state->labels, tokval);
      if (i == NOTFOUND || i >= read_state->exprs.items->vector.fillp ||
          read_state->exprs.items->vector.self[i] == LISP_UNBOUND) {
        LispPrintStr("read: error: undefined label ");
        LispPrintObject(tokval, false);
        LispError("\n");
      }
      v = read_state->exprs.items->vector.self[i];
      break;
    }
    default:
      break;
  }
  if (list_p) {
    v = MakeCons();
    LISP_CONS_CAR(v) = head;
    LISP_CONS_CDR(v) = MakeCons();
    LISP_CONS_CAR(LISP_CONS_CDR(v)) = LISP_CONS_CDR(LISP_CONS_CDR(v)) =
        LISP_NIL;
    PUSH(v);
    if (fixup != NOTFOUND) {
      read_state->exprs.items->vector.self[fixup] = v;
    }
    v = do_read_sexpr(NOTFOUND);
    LISP_CONS_CAR(LISP_CONS_CDR(stack[stack_index - 1])) = v;
    v = POP();
  }
  return v;
}
LispObject ReadSexpr() {
  /* static structs */
  /* struct LispVector8 {  /\*  vector header  *\/ */
  /*   _LISP_HDR;          /\*  array element type*\/ */
  /*   LispIndex size;     /\*  dimension  *\/ */
  /*   LispIndex fillp;    /\*  fill pointer  *\/ */
  /*                       /\*  For simple vectors, fillp is equal to dim. *\/ */
  /*   LispObject self[8]; /\*  pointer to the vector  *\/ */
  /* }; */
  /* alignas(ALIGN_TYPE) struct LispVector8 vec1; */
  /* alignas(ALIGN_TYPE) struct LispVector8 vec2; */

  LispObject v;
  ReadState state;
  state.prev = read_state;
  LabelTableInit(&state.labels, 8);
  LabelTableInit(&state.exprs, 8);
  /* FIXME: check labels are working correctly */
  /* state.labels.items = (LispObject)&vec1; */
  /* state.exprs.items = (LispObject)&vec2; */
  read_state = &state;

  v = do_read_sexpr(NOTFOUND);

  read_state = state.prev;
  return v;
}
