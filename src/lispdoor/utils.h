/**
 *   \file utils.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_UTILS_H_INCLUDED
#define LISPDOOR_UTILS_H_INCLUDED

#include "lispdoor/memorylayout.h"
#include "lispdoor/objects.h"
#include "lispdoor/print.h"

#define NOTFOUND ((LispIndex)-1)
#define UEOF ((uint32_t)EOF)

char *Int2Str(char *dest, LispIndex len, int32_t num, Byte base);
char *Uint2Str(char *dest, LispIndex len, uint32_t num, Byte base);
char *Float2Str(char *str, LispIndex len, float f, uint8_t precision);
char *Double2Str(char *str, LispIndex len, double f, uint8_t precision);
void LabelTableInit(LabelTable *t, LispIndex n);
void LabelTableClear(LabelTable *t);
void LabelTableInsert(LabelTable *t, LispObject item);
LispIndex LabelTableLookUp(LabelTable *t, LispObject item);
void LabelTableAdjoin(LabelTable *t, LispObject item);
void LabelTableRelocate(LabelTable *t);

static inline void ArgCount(char *fname, LispIndex nargs, LispIndex c) {
  if (nargs != c) {
    LispPrintStr(fname);
    LispPrintStr(": error: wrong number of arguments expected ");
    LispPrintStr(
        Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, c, lisp_number_base));
    LispPrintStr(" got ");
    LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, nargs,
                          lisp_number_base));
    LispError("\n");
  }
}

#endif /* LISPDOOR_UTILS_H_INCLUDED */
