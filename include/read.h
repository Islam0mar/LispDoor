/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef READ_H
#define READ_H

#include "objects.h"

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

int symchar(char c);

/* get char */
int32_t ReadChar();
int32_t GetChar();
void UnGetChar();

char nextchar(FILE *f);

void take(void);

void accumchar(char c, int *pi);

// return: 1 for dot token, 0 for symbol
int read_token(FILE *f, char c, int digits);
u_int32_t peek(FILE *f);

/* Parser                   tokens --> ast */
void read_list(FILE *f, LispObject *pval, LispFixNum fixup);
LispObject do_read_sexpr(FILE *f, LispFixNum fixup);
LispObject ReadSexpr(FILE *f);

#endif /* READ_H */
