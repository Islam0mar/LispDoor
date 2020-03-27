/**
 *   \file print.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef PRINT_H
#define PRINT_H

#include "objects.h"
#include "read.h"

extern LispObject cons_flags;
extern LabelTable print_conses;

/* print */
void LispPrint(FILE *f, LispObject v, int princ);

/* error utilities  */
void LispError(char *format, ...);
void LispTypeError(char *fname, char *expected, LispObject got);

#endif /* PRINT_H */
