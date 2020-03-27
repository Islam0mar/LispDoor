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

#include "memorylayout.h"

LispObject cons_flags;
LabelTable print_conses;

#define CONS_INDEX(c) \
  (((LispFixNum)LISP_CONS_PTR(c)) - ((LispFixNum)from_space))

#define MARKED_P(c) LispBitVectorGet(cons_flags, CONS_INDEX(c))
#define MARK_CONS(c) LispBitVectorSet(cons_flags, CONS_INDEX(c), 1)
#define UNMARK_CONS(c) LispBitVectorSet(cons_flags, CONS_INDEX(c), 0)

// error utilities ------------------------------------------------------------
void LispError(char *format, ...) {
  va_list args;
  va_start(args, format);
  read_state = NULL;
  vfprintf(stderr, format, args);
  va_end(args);
  longjmp(LispEnv()->top_level, 1);
}
void LispTypeError(char *fname, char *expected, LispObject got) {
  fprintf(stderr, "%s: error: expected %s, got ", fname, expected);
  LispPrint(stderr, got, 0);
  LispError("\n");
}

static void PrintTraverse(LispObject v) {
  while (LISP_ConsP(v)) {
    if (MARKED_P(v)) {
      LabelTableAdjoin(&print_conses, v);
      break;
    }
    MARK_CONS(v);
    PrintTraverse(LISP_CONS_CAR(v));
    v = LISP_CONS_CDR(v);
  }
}

static void PrintSymbol(FILE *f, char *name) {
  int i, escape = 0, charescape = 0;

  if (name[0] == '\0') {
    fprintf(f, "||");
    return;
  }
  if (name[0] == '.' && name[1] == '\0') {
    fprintf(f, "|.|");
    return;
  }
  if (name[0] == '#') escape = 1;
  i = 0;
  while (name[i]) {
    if (!symchar(name[i])) {
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
      fprintf(f, "|");
      i = 0;
      while (name[i]) {
        if (name[i] == '|' || name[i] == '\\')
          fprintf(f, "\\%c", name[i]);
        else
          fprintf(f, "%c", name[i]);
        i++;
      }
      fprintf(f, "|");
    } else {
      fprintf(f, "|%s|", name);
    }
  } else {
    fprintf(f, "%s", name);
  }
}

static void DoPrint(FILE *f, LispObject o, int princ) {
  LispObject cd;
  LispFixNum label;
  char *name;
  if (LISP_UNBOUNDP(o)) {
    fprintf(f, "unbound");
  } else if (LISP_NULL(o)) {
    fprintf(f, "nil");
  } else if (o == LISP_T) {
    fprintf(f, "t");
  } else {
    switch (LISP_TYPE_OF(o)) {
      case kFixNum: {
        fprintf(f, "%ld", LISP_FIXNUM(o));
        break;
      }
      case kCharacter: {
        if (!setlocale(LC_CTYPE, "")) {
          LispError(
              "Can't set the specified locale! "
              "Check LANG, LC_CTYPE, LC_ALL.\n");
        }
        fprintf(f, "#\\%lc", LISP_CHAR_CODE(o));
        break;
      }
      case kLongFloat: {
        fprintf(f, "%LF", LISP_LONG_FLOAT(o));
        break;
      }
      case kSingleFloat: {
        fprintf(f, "%f", LISP_SINGLE_FLOAT(o));
        break;
      }
      case kDoubleFloat: {
        fprintf(f, "%f", LISP_DOUBLE_FLOAT(o));
        break;
      }
      case kString: {
        fprintf(f, "\"%.*s\"", (int)o->string.size, o->string.self);
        break;
      }
      case kBitVector: {
        fprintf(f, "#b(");
        label = 0;
        for (label = 0;;) {
          fprintf(f, "%#x", o->bit_vector.self[label]);
          if (++label < o->bit_vector.size) {
            fprintf(f, " ");
          } else {
            break;
          }
        }
        fprintf(f, ")");
        break;
      }
      case kForwarded: {
        fprintf(f, "{FORWARDED: ");
        DoPrint(f, LISP_FORWARD(o), princ);
        fprintf(f, "}");
        break;
      }
      case kCFunction: {
        fprintf(f, "#.");
        if (princ) {
          fprintf(f, "%s", o->cfun.name);
        } else {
          PrintSymbol(f, o->cfun.name);
        }
        break;
      }
      case kSymbol: {
        if (princ) {
          fprintf(f, "%s", LispSymbolName(o));
        } else {
          PrintSymbol(f, LispSymbolName(o));
        }
        break;
      }
      case kList: {
        if ((label = LabelTableLookUp(&print_conses, o)) != NOTFOUND) {
          if (!MARKED_P(o)) {
            fprintf(f, "#%ld#", label);
            break;
          }
          fprintf(f, "#%ld=", label);
        }
        fprintf(f, "(");
        while (1) {
          UNMARK_CONS(o);
          DoPrint(f, LISP_CONS_CAR(o), princ);
          cd = LISP_CONS_CDR(o);
          if (!LISP_ConsP(cd)) {
            if (cd != LISP_NIL) {
              fprintf(f, " . ");
              DoPrint(f, cd, princ);
            }
            fprintf(f, ")");
            break;
          } else {
            if ((label = LabelTableLookUp(&print_conses, cd)) != NOTFOUND) {
              fprintf(f, " . ");
              DoPrint(f, cd, princ);
              fprintf(f, ")");
              break;
            }
          }
          fprintf(f, " ");
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

void LispPrint(FILE *f, LispObject v, int princ) {
  LabelTableClear(&print_conses);
  PrintTraverse(v);
  DoPrint(f, v, princ);
}
