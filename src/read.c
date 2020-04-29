/**
 *   \file read.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "read.h"

#include "eval.h"
#include "memorylayout.h"
#include "print.h"
#include "read.h"
#include "symboltree.h"
#include "utils.h"

enum {
  kTokNone,
  kTokOpen,
  kTokClose,
  kTokDot,
  kTokQuote,
  kTokSym,
  kTokNum,
  kTokSingleFloat,
  kTokDoubleFloat,
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
};

ReadState *read_state = NULL;

LispIndex toktype = kTokNone;
LispObject tokval;

static const uint32_t offsets_from_utf8[6] = {0x00000000UL, 0x00003080UL,
                                              0x000E2080UL, 0x03C82080UL,
                                              0xFA082080UL, 0x82082080UL};

static const char trailing_bytes_for_utf8[256] = {
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

/* get char */
static inline int32_t ReadChar(FILE *f) { return fgetc(f); }
int32_t GetChar(FILE *f) {
  int32_t c;
  if (chars_buf_index_start == chars_buf_index_end) {
    c = ReadChar(f);
    chars_buf[chars_buf_index_end++] = c;
    chars_buf_index_end = chars_buf_index_end % CHARS_BUF_SIZE;
  }
  c = chars_buf[chars_buf_index_start++];
  chars_buf_index_start = chars_buf_index_start % CHARS_BUF_SIZE;
  return c;
}
void UnGetChar() {
  if (chars_buf_index_start == 0) {
    chars_buf_index_start = CHARS_BUF_SIZE - 1;
  } else {
    chars_buf_index_start--;
  }
}

int32_t UTF8SeqLen(const char c) {
  return trailing_bytes_for_utf8[(unsigned int)(unsigned char)c] + 1;
}

uint32_t UTF8GetChar(FILE *f) {
  int32_t amt = 0, sz, c;
  uint32_t ch = 0;

  c = GetChar(f);
  if (c == EOF) return UEOF;
  ch = (uint32_t)c;
  amt = sz = UTF8SeqLen(ch);
  while (--amt) {
    ch <<= 6;
    c = GetChar(f);
    if (c == EOF) return UEOF;
    ch += (uint32_t)c;
  }
  ch -= offsets_from_utf8[sz - 1];

  return ch;
}

char nextchar(FILE *f) {
  char c;
  int32_t ch;

  do {
    ch = GetChar(f);
    if (ch == EOF) return 0;
    c = (char)ch;
    if (c == ';') {
      // single-line comment
      do {
        ch = GetChar(f);
        if (ch == EOF) return 0;
      } while ((char)ch != '\n');
      c = (char)ch;
    }
  } while (isspace(c));
  return c;
}

void take(void) { toktype = kTokNone; }

void accumchar(char c, int32_t *pi) {
  buf[(*pi)++] = c;
  if (*pi >= (int32_t)(sizeof(buf) - 1))
    LispError("read: error: token too long\n");
}

// return: 1 for dot token, 0 for symbol
/* return: true for dot token, false for symbol */
bool read_token(FILE *f, char c, bool digits) {
  int32_t i = 0, ch, escaped = 0, dot = (c == '.'), totread = 0;

  UnGetChar();
  while (1) {
    ch = GetChar(f);
    totread++;
    if (ch == EOF) goto terminate;
    c = (char)ch;
    if (c == '|') {
      escaped = !escaped;
    } else if (c == '\\') {
      ch = GetChar(f);
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
  buf[i++] = '\0';
  return (dot && (totread == 2));
}

LispIndex peek(FILE *f) {
  char c, *end;
  LispFixNum x;

  if (toktype != kTokNone) return toktype;
  c = nextchar(f);
  if (feof(f)) return kTokNone;
  if (c == '(') {
    toktype = kTokOpen;
  } else if (c == ')') {
    toktype = kTokClose;
  } else if (c == '\'') {
    toktype = kTokQuote;
  } else if (c == '`') {
    toktype = kTokBackQuote;
  } else if (c == '#') {
    c = GetChar(f);
    if (c == EOF) LispError("read: error: invalid read macro\n");
    if ((char)c == '.') {
      toktype = kTokSharpDot;
    } else if ((char)c == '\'') {
      toktype = kTokSharpQuote;
    } else if ((char)c == '\\') {
      /* FixMe: */
      toktype = kTokNum;
      tokval = LISP_MAKE_CHARACTER(UTF8GetChar(f));
    } else if (isdigit((char)c)) {
      read_token(f, (char)c, true);
      c = (char)GetChar(f);
      if (c == '#')
        toktype = kTokBackRef;
      else if (c == '=')
        toktype = kTokLabel;
      else
        LispError("read: error: invalid label\n");
      x = strtol(buf, &end, 10);
      tokval = LISP_MAKE_FIXNUM(x);
    } else {
      LispError("read: error: unknown read macro\n");
    }
  } else if (c == ',') {
    toktype = kTokComma;
    c = GetChar(f);
    if (c == EOF) return toktype;
    if ((char)c == '@')
      toktype = kTokCommaAt;
    else if ((char)c == '.')
      toktype = kTokCommaDot;
    else
      UnGetChar();
  } else if (isdigit(c) || c == '-' || c == '+') {
    read_token(f, c, false);
    if (c == '0' && (buf[1] == 'b')) {
      x = strtol(buf + 2, &end, 2);
    } else {
      x = strtol(buf, &end, 0);
    }
    if (*end != '\0') {
      toktype = kTokSym;
      tokval = LispMakeSymbol(buf);
    } else {
      toktype = kTokNum;
      tokval = LISP_MAKE_FIXNUM(x);
    }
  } else {
    if (read_token(f, c, false)) {
      toktype = kTokDot;
    } else {
      toktype = kTokSym;
      tokval = LispMakeSymbol(buf);
    }
  }
  return toktype;
}

/* Parser                   tokens --> ast */
LispObject do_read_sexpr(FILE *f, LispFixNum fixup);
// build a list of conses. this is complicated by the fact that all conses
// can move whenever a new cons is allocated. we have to refer to every cons
// through a handle to a relocatable pointer (i.e. a pointer on the stack).
void read_list(FILE *f, LispObject *pval, LispFixNum fixup) {
  LispObject c, *pc;
  uint32_t t;

  PUSH(LISP_NIL);
  pc = &stack[stack_ptr - 1];  // to keep track of current cons cell
  t = peek(f);
  while (t != kTokClose) {
    if (feof(f)) {
      LispError("read: error: unexpected end of input\n");
    }
    c = MakeCons();
    LISP_CONS_CAR(c) = LISP_CONS_CDR(c) = LISP_NIL;
    if (LISP_ConsP(*pc)) {
      LISP_CONS_CDR(*pc) = c;
    } else {
      *pval = c;
      if (fixup != -1) {
        read_state->exprs.items[fixup] = c;
      }
    }
    *pc = c;
    c = do_read_sexpr(f, -1);  // must be on separate lines due to undefined
    LISP_CONS_CAR(*pc) = c;    // evaluation order

    t = peek(f);
    if (t == kTokDot) {
      take();
      c = do_read_sexpr(f, -1);
      LISP_CONS_CDR(*pc) = c;
      t = peek(f);
      if (feof(f)) {
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
LispObject do_read_sexpr(FILE *f, LispFixNum fixup) {
  LispObject v = LISP_NIL, head;
  LispIndex i;
  uint32_t t = peek(f);
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
      head = LispMakeSymbol(",");
      list_p = true;
      break;
    }
    case kTokCommaAt: {
      head = LispMakeSymbol(",@");
      list_p = true;
      break;
    }
    case kTokCommaDot: {
      head = LispMakeSymbol(",.");
      list_p = true;
      break;
    }
    case kTokBackQuote: {
      head = LispMakeSymbol("`");
      list_p = true;
      break;
    }
    case kTokQuote: {
      head = LispMakeSymbol("quote");
      list_p = true;
      break;
    }
    case kTokSharpQuote: {
      v = do_read_sexpr(f, fixup);
      break;
    }
    case kTokOpen: {
      PUSH(LISP_NIL);
      read_list(f, &stack[stack_ptr - 1], fixup);
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
      v = do_read_sexpr(f, -1);
      v = TopLevelEval(v);
      break;
    }
    case kTokLabel: {
      /* create backreference label */
      if (LabelTableLookUp(&read_state->labels, tokval) != NOTFOUND) {
        LispError("read: error: label %d redefined\n", LISP_FIXNUM(tokval));
      }
      LabelTableInsert(&read_state->labels, tokval);
      i = read_state->exprs.n;
      LabelTableInsert(&read_state->exprs, LISP_UNBOUND);
      v = do_read_sexpr(f, (LispFixNum)i);
      read_state->exprs.items[i] = v;
      break;
    }
    case kTokBackRef: {
      /* look up backreference */
      i = LabelTableLookUp(&read_state->labels, tokval);
      if (i == NOTFOUND || i >= (int32_t)read_state->exprs.n ||
          read_state->exprs.items[i] == LISP_UNBOUND) {
        LispError("read: error: undefined label %d\n", LISP_FIXNUM(tokval));
      }
      v = read_state->exprs.items[i];
      break;
    }
    default:
      break;
  }
  if (list_p) {
    v = ConsReserve(2);
    LISP_CONS_CAR(v) = head;
    LISP_CONS_CDR(v) = LISP_PTR_CONS(LISP_CONS_PTR(v) + 1);
    LISP_CONS_CAR(LISP_CONS_CDR(v)) = LISP_CONS_CDR(LISP_CONS_CDR(v)) =
        LISP_NIL;
    PUSH(v);
    if (fixup != -1) {
      read_state->exprs.items[fixup] = v;
    }
    v = do_read_sexpr(f, -1);
    LISP_CONS_CAR(LISP_CONS_CDR(stack[stack_ptr - 1])) = v;
    v = POP();
  }
  return v;
}
LispObject ReadSexpr(FILE *f) {
  LispObject v;
  ReadState state;
  state.prev = read_state;
  LabelTableInit(&state.labels, 16);
  LabelTableInit(&state.exprs, 16);
  read_state = &state;

  v = do_read_sexpr(f, -1);

  read_state = state.prev;
  return v;
}

LispObject LoadFile(char *fname) {
  LispObject e, v = LISP_NIL;
  FILE *f = fopen(fname, "r");
  LispIndex i = 0;
  if (f == NULL) {
    LispError("file not found\n");
  }
  while (1) {
    e = ReadSexpr(f);
    printf("%4ld.", i++);
    LispPrint(stdout, e, 0);
    printf("\n");
    if (feof(f)) {
      break;
    }
    e = TopLevelEval(e);
  }
  fclose(f);
  return v;
}
