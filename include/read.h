/**
 *   \file read.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef READ_H
#define READ_H

#include "objects.h"

/* Lexical analyzer         input string --> tokens */
/* read */
#define NOTFOUND ((LispFixNum)-1)
#define UEOF ((uint32_t)EOF)

typedef struct {
  LispIndex n, maxsize;
  LispObject *items;
} LabelTable;

typedef struct _ReadState {
  LabelTable labels;
  LabelTable exprs;
  struct _ReadState *prev;
} ReadState;

extern ReadState *read_state;

bool SymCharP(char c);
LispObject ReadSexpr(FILE *f);
LispObject LoadFile(char *fname);

#endif /* READ_H */
