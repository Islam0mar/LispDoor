/*
 *    \file symboltree.c
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

#include "lispdoor/symboltree.h"

#include <stdlib.h>

#include "lispdoor/gc.h"
#include "lispdoor/objects.h"
#include "lispdoor/print.h"

static inline int32_t SymbolArrayCompare(const void *a, const void *b) {
  return strcmp(((LispObject)a)->symbol.name, ((LispObject)b)->symbol.name);
}

/* A iterative binary search function. It returns */
/* location of x in given array arr[l..r] if present, */
/* otherwise -1 */
int32_t SymbolArrayBinarySearch(LispObject arr[], int32_t l, int32_t r,
                                char name[]) {
  while (l <= r) {
    int32_t m = l + (r - l) / 2;

    int32_t x = strcmp(name, arr[m]->symbol.name);
    /* Check if name is present at mid */
    if (x == 0) {
      return m;
    }
    /* If name greater, ignore left half */
    else if (x > 0) {
      l = m + 1;
    }
    /* If name is smaller, ignore right half */
    else {
      r = m - 1;
    }
  }

  /* if we reach here, then element was */
  /* not present */
  return -1;
}

static inline int32_t SymbolArrayLookUp(LispObject vec, char name[]) {
  return SymbolArrayBinarySearch(vec->vector.self, 0, vec->vector.fillp - 1,
                                 name);
}

void SymbolArraySwap(LispObject a[], int32_t n, int32_t m) {
  if (n != m) {
    LispObject tmp = a[n];
    a[n] = a[m];
    a[m] = tmp;
  }
}

void SymbolArrayQuickSort(LispObject a[], int32_t n) {
  while (n > 1) {
    int32_t i;
    int32_t last = 0;
    SymbolArraySwap(a, 0, n / 2);
    for (i = 1; i < n; i++) {
      if (SymbolArrayCompare(a[i], a[0]) < 0) {
        SymbolArraySwap(a, ++last, i);
      }
    }
    SymbolArraySwap(a, 0, last);
    /* TCO */
    if (last > n - last - 1) {
      SymbolArrayQuickSort(a + last + 1, n - last - 1);
      n = last;
    } else {
      SymbolArrayQuickSort(a, last);
      a = a + last + 1;
      n = n - last - 1;
    }
  }
}

LispObject LispMakeSymbol(char *str) {
  int32_t index;
  LispObject *symbols_vector = &LispEnv()->symbols;

  index = SymbolArrayLookUp(*symbols_vector, str);
  if (index == -1) {
    LispObject sym = LispAllocObject(kSymbol, (LispIndex)strlen(str));
    sym->symbol.value = LISP_UNBOUND;
    sym->symbol.stype = kSymOrdinary;
    strcpy(sym->symbol.name, str);
    *symbols_vector = LispVectorPush(*symbols_vector, sym);
    SymbolArrayQuickSort((*symbols_vector)->vector.self,
                         (*symbols_vector)->vector.fillp);
    index = SymbolArrayLookUp(*symbols_vector, str);
  }
  return (*symbols_vector)->vector.self[index];
}
