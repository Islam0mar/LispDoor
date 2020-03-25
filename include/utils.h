/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include "objects.h"

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

static ReadState *read_state = NULL;

char *Uint2Str(char *dest, LispIndex len, LispIndex num, Byte base);

void LabelTableInit(LabelTable *t, LispIndex n);

void LabelTableClear(LabelTable *t);

void LabelTableInsert(LabelTable *t, LispObject item);

LispFixNum LabelTableLookUp(LabelTable *t, LispObject item);

void LabelTableAdjoin(LabelTable *t, LispObject item);

int u8_seqlen(const char c);

uint32_t UTF8GetChar(FILE *f);
void argcount(char *fname, LispIndex nargs, LispIndex c);
static inline LispIndex ConsCount(LispObject v);
static inline LispObject ConsNth(LispIndex n, LispObject lst);

#endif /* UTILS_H */
