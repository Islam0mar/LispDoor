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
void LispPrintObject(LispObject v, uint8_t princ);
void LispPrintStr(char *str);
void LispPrintStrN(char *s, uint16_t len);
void LispPrintByte(uint8_t c);

/* error utilities  */
void LispError(char *format);
void LispTypeError(char *fname, char *expected, LispObject got);

#endif /* LISPDOOR_PRINT_H_INCLUDED */
