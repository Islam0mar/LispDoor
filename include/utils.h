/**
 *   \file utils.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include "objects.h"
#include "print.h"

char *Uint2Str(char *dest, LispIndex len, LispIndex num, Byte base);
void LabelTableInit(LabelTable *t, LispIndex n);
void LabelTableClear(LabelTable *t);
void LabelTableInsert(LabelTable *t, LispObject item);
LispFixNum LabelTableLookUp(LabelTable *t, LispObject item);
void LabelTableAdjoin(LabelTable *t, LispObject item);

static inline void ArgCount(char *fname, LispIndex nargs, LispIndex c) {
  if (nargs != c) {
    LispError("%s: error: too %s arguments\n", fname,
              nargs < c ? "few" : "many");
  }
}

#endif /* UTILS_H */
