/**
 *   \file symboltree.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_SYMBOLTREE_H_INCLUDED
#define LISPDOOR_SYMBOLTREE_H_INCLUDED

#include "lispdoor/objects.h"

/* COUNT specify spaces to print for symbol tree(AVL) */
/* COUNT distance between levels */
#define COUNT 3
void SymbolTreePrint2D(struct LispSymbol *root, int space);
LispObject LispMakeSymbol(char *str);

#endif /* LISPDOOR_SYMBOLTREE_H_INCLUDED */
