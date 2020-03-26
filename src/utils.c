/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "utils.h"

#include "gc.h"
#include "print.h"

char *Uint2Str(char *dest, LispIndex len, LispIndex num, Byte base) {
  LispIndex i = len - 1;
  LispIndex b = (LispIndex)base;
  char ch;
  dest[i--] = '\0';
  while (i >= 0) {
    ch = (char)(num % b);
    if (ch < 10)
      ch += '0';
    else
      ch = ch - 10 + 'a';
    dest[i--] = ch;
    num /= b;
    if (num == 0) break;
  }
  return &dest[i + 1];
}

void LabelTableInit(LabelTable *t, LispIndex n) {
  t->n = 0;
  t->maxsize = n;
  t->items = (LispObject *)GcMalloc(n * sizeof(LispObject));
}

void LabelTableClear(LabelTable *t) { t->n = 0; }

void LabelTableInsert(LabelTable *t, LispObject item) {
  LispObject *p;
  if (t->n == t->maxsize) {
    p = GcMalloc((t->maxsize * 2) * sizeof(LispObject));
    LispIndex i;
    for (i = 0; i < t->maxsize; ++i) {
      p[i] = t->items[i];
    }
    t->items = p;
    t->maxsize *= 2;
  }
  t->items[t->n++] = item;
}

LispFixNum LabelTableLookUp(LabelTable *t, LispObject item) {
  LispFixNum i;
  for (i = 0; i < (LispFixNum)t->n; i++) {
    if (t->items[i] == item) {
      return i;
    }
  }
  return NOTFOUND;
}

void LabelTableAdjoin(LabelTable *t, LispObject item) {
  if (LabelTableLookUp(t, item) == NOTFOUND) {
    LabelTableInsert(t, item);
  }
}


