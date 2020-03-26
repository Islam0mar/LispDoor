/**
 *   \file symboltree.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef SYMBOLTREE_H
#define SYMBOLTREE_H

#include "objects.h"

/* COUNT specify spaces to print for symbol tree(AVL) */
/* COUNT distance between levels */
#define COUNT 3
void SymbolTreePrint2D(struct LispSymbol *root, int space);
LispObject LispMakeSymbol(char *str);

#endif /* SYMBOLTREE_H */
