/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "utils.h"
#include "print.h"
#include "gc.h"

static const uint32_t offsetsFromUTF8[6] = {0x00000000UL, 0x00003080UL,
                                            0x000E2080UL, 0x03C82080UL,
                                            0xFA082080UL, 0x82082080UL};

static const char trailingBytesForUTF8[256] = {
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

int u8_seqlen(const char c) {
  return trailingBytesForUTF8[(unsigned int)(unsigned char)c] + 1;
}

uint32_t UTF8GetChar(FILE *f) {
  int amt = 0, sz, c;
  uint32_t ch = 0;

  c = GetChar();
  if (c == EOF) return UEOF;
  ch = (uint32_t)c;
  amt = sz = u8_seqlen(ch);
  while (--amt) {
    ch <<= 6;
    c = GetChar();
    if (c == EOF) return UEOF;
    ch += (uint32_t)c;
  }
  ch -= offsetsFromUTF8[sz - 1];

  return ch;
}

