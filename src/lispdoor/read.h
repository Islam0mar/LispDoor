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

/* Lexical analyzer         input string --> tokens */
/* read */

bool SymCharP(char c);
LispObject ReadSexpr();

#endif /* LISPDOOR_READ_H_INCLUDED */
