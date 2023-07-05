/*
 *    \file utils.c
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
  t->items = LispMakeVector(n);
}

void LabelTableClear(LabelTable *t) { t->items->vector.fillp = 0; }

void LabelTableInsert(LabelTable *t, LispObject item) {
  LispVectorPush(t->items, item);
}

LispIndex LabelTableLookUp(LabelTable *t, LispObject item) {
  LispIndex i;
  for (i = 0; i < t->items->vector.size; i++) {
    if (t->items->vector.self[i] == item) {
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
