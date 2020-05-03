/**
 *   \file read.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_READ_H_INCLUDED
#define LISPDOOR_READ_H_INCLUDED

#include "lispdoor/objects.h"
#include "lispdoor/utils.h"

#define UEOF ((uint32_t)-1)
#define EOF ((uint8_t)-1)

/* Lexical analyzer         input string --> tokens */
LispObject ReadSexpr();
bool SymCharP(char c);

#endif /* LISPDOOR_READ_H_INCLUDED */
