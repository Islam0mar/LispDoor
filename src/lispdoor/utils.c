/**
 *   \file utils.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "lispdoor/utils.h"

#include "lispdoor/gc.h"
#include "lispdoor/memorylayout.h"
#include "lispdoor/print.h"
/* largest number for double is 2^53 = 9 * 10^15. */
uint8_t Log10(uint64_t v) {
  return (v >= 10000000000000000u)
             ? 16
             : (v >= 1000000000000000u)
                   ? 15
                   : (v >= 100000000000000u)
                         ? 14
                         : (v >= 10000000000000u)
                               ? 13
                               : (v >= 1000000000000u)
                                     ? 12
                                     : (v >= 100000000000u)
                                           ? 11
                                           : (v >= 10000000000u)
                                                 ? 10
                                                 : (v >= 1000000000u)
                                                       ? 9
                                                       : (v >= 100000000u)
                                                             ? 8
                                                             : (v >= 10000000u)
                                                                   ? 7
                                                                   : (v >=
                                                                      1000000u)
                                                                         ? 6
                                                                         : (v >=
                                                                            100000u)
                                                                               ? 5
                                                                               : (v >=
                                                                                  10000u)
                                                                                     ? 4
                                                                                     : (v >=
                                                                                        1000u)
                                                                                           ? 3
                                                                                           : (v >=
                                                                                              100u)
                                                                                                 ? 2
                                                                                                 : (v >=
                                                                                                    10u)
                                                                                                       ? 1u
                                                                                                       : 0u;
}
char *Uint642Str(char *dest, LispIndex len, uint64_t num) {
  LispFixNum i = (LispFixNum)len - 1;
  char ch;
  dest[i--] = '\0';
  while (i >= 0) {
    ch = (char)(num % 10);
    if (ch < 10)
      ch += '0';
    else
      ch = ch - 10 + 'a';
    dest[i--] = ch;
    num /= 10;
    if (num == 0) break;
  }
  if (num != 0) {
    LispPrintStr("warning: number trancated\n");
  }
  return &dest[i + 1];
}
char *Double2Str(char *str, LispIndex len, double f, uint8_t precision) {
  uint8_t i, digit;
  const double epsilon = 1e-5;
  bool is_negative = f < 0;
  if (is_negative) {
    f = -f;
  }
  /* split f to integer and fraction */
  uint64_t integer_part = (uint64_t)f;
  f -= (double)integer_part;
  uint8_t number_of_digits = 1 + Log10(integer_part);
  if (is_negative) {
    str[0] = '-';
    str++;
    len--;
  }
  /* write decimal value to string */
  if (++number_of_digits > len) {
    LispError("error: scratch pad overflow\n");
  }
  Uint642Str(str, number_of_digits, integer_part);
  str[number_of_digits - 1] = precision == 0 ? '\0' : '.';
  for (i = 0; i < precision; ++i) {
    if (number_of_digits + i >= len - 1) {
      LispPrintStr("warning: double floating number trancated\n");
      break;
    }
    f *= 10;
    integer_part = (uint64_t)f;
    digit = (uint8_t)(integer_part % 10);
    str[number_of_digits + i] = '0' + digit;
    f -= (double)integer_part;
    if (f < epsilon) {
      break;
    }
  }
  str[number_of_digits + i] = '\0';
  return is_negative ? --str : str;
}
char *Float2Str(char *str, LispIndex len, float f, uint8_t precision) {
  uint8_t i, digit;
  const float epsilon = (float)1e-5;
  bool is_negative = f < 0;
  if (is_negative) {
    f = -f;
  }
  /* split f to integer and fraction */
  uint32_t integer_part = (uint32_t)f;
  f -= (float)integer_part;
  uint8_t number_of_digits = 1 + Log10(integer_part);
  if (is_negative) {
    str[0] = '-';
    str++;
    len--;
  }
  /* write decimal value to string */
  if (++number_of_digits > len) {
    LispError("error: scratch pad overflow\n");
  }
  Uint2Str(str, number_of_digits, integer_part, 10);
  str[number_of_digits - 1] = precision == 0 ? '\0' : '.';
  for (i = 0; i < precision; ++i) {
    if (number_of_digits + i >= len - 1) {
      LispPrintStr("warning: floating number trancated\n");
      break;
    }
    f *= 10;
    integer_part = (uint32_t)f;
    digit = (uint8_t)(integer_part % 10);
    str[number_of_digits + i] = '0' + digit;
    f -= (float)integer_part;
    if (f < epsilon) {
      break;
    }
  }
  str[number_of_digits + i] = '\0';
  return is_negative ? --str : str;
}
char *Int2Str(char *dest, LispIndex len, int32_t num, Byte base) {
  int16_t i = (int16_t)len - 1;
  bool is_negative = num < 0;
  if (is_negative) {
    num = -num;
  }
  char ch;
  dest[i--] = '\0';
  while (i >= 0) {
    ch = (char)(num % base);
    if (ch < 10)
      ch += '0';
    else
      ch = ch - 10 + 'a';
    dest[i--] = ch;
    num /= base;
    if (num == 0) break;
  }
  if (num != 0) {
    LispPrintStr("warning: number trancated\n");
  }
  if (is_negative) {
    if (i >= 0) {
      dest[i--] = '-';
    } else {
      LispPrintStr("warning: negative sign trancated\n");
    }
  }
  return &dest[i + 1];
}
char *Uint2Str(char *dest, LispIndex len, uint32_t num, Byte base) {
  int16_t i = (int16_t)len - 1;
  char ch;
  dest[i--] = '\0';
  while (i >= 0) {
    ch = (char)(num % base);
    if (ch < 10)
      ch += '0';
    else
      ch = ch - 10 + 'a';
    dest[i--] = ch;
    num /= base;
    if (num == 0) break;
  }
  if (num != 0) {
    LispPrintStr("warning: number trancated\n");
  }
  return &dest[i + 1];
}

void LabelTableInit(LabelTable *t, LispIndex n) {
  t->n = 0;
  t->maxsize = n;
  t->items = (LispObject *)GcMalloc(n * sizeof(LispObject));
}

void LabelTableRelocate(LabelTable *t) {
  t->items = (LispObject *)GcMalloc(t->maxsize * sizeof(LispObject));
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

LispIndex LabelTableLookUp(LabelTable *t, LispObject item) {
  LispIndex i;
  for (i = 0; i < t->n; i++) {
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
