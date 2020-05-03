/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

/* used for wchar printing */
#include "print.h"

#include <locale.h>
#include <wchar.h>

#include "hal/bsp.h"
#include "lispdoor/memorylayout.h"
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
  LispPrintObject(got, 0);
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

static void DoPrint(LispObject o, int princ) {
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
        assert(false);
    }
  }
}

void LispPrintObject(LispObject v, uint8_t princ) {
  LabelTableClear(&print_conses);
  PUSH(v);
  PrintTraverse(v);
  v = POP();
  DoPrint(v, princ);
}
void LispPrintStr(char *str) { UART1_SendStr(str); }
void LispPrintByte(uint8_t c) { UART1_SendByte(c); }
void LispPrintStrN(char *s, uint16_t len) { UART1_SendStrN(s, len); }
