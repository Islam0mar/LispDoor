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

enum {
  kTokNone,
  kTokOpen,
  kTokClose,
  kTokDot,
  kTokQuote,
  kTokSym,
  kTokNum,
  kTokSingleFloat,
  kTokDoubleFloat,
  kTokBackQuote,
  kTokComma,
  kTokCommaAt,
  kTokCommaDot,
  kTokSharpDot,
  kTokLabel,
  kTokBackRef,
  kTokSharpQuote,
  kTokSharpOpen,
  kTokOpenB,
  kTokCloseB,
  kTokSharpSym,
  kTokGenSym,
  kTokDoubleQuote
};

bool symchar(char c);

/* get char */
int32_t ReadChar();
int32_t GetChar();
void UnGetChar();
char nextchar(FILE *f);
void take(void);
void accumchar(char c, int32_t *pi);
int32_t read_token(FILE *f, char c, int32_t digits);
uint32_t peek(FILE *f);

/* Parser                   tokens --> ast */
void read_list(FILE *f, LispObject *pval, LispFixNum fixup);
LispObject do_read_sexpr(FILE *f, LispFixNum fixup);
LispObject ReadSexpr(FILE *f);

#endif /* READ_H */
