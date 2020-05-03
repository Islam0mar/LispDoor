/**
 *   \file print.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_PRINT_H_INCLUDED
#define LISPDOOR_PRINT_H_INCLUDED

#include "lispdoor/objects.h"

/* print */
void LispPrintObject(LispObject v, bool princ);
void LispPrintStr(char *str);
void LispPrintStrN(char *s, LispIndex len);
void LispPrintByte(Byte c);

/* error utilities  */
void LispError(char *format);
void LispTypeError(char *fname, char *expected, LispObject got);

#endif /* LISPDOOR_PRINT_H_INCLUDED */
