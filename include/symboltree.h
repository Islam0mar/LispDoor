/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef SYMBOLTREE_H
#define SYMBOLTREE_H

#include "objects.h"

#define COUNT 3
// Function to print binary tree in 2D
// It does reverse inorder traversal
void print2DUtil(struct LispSymbol *root, int space);
struct LispSymbol *SymTableLookUp(struct LispSymbol *ptree, char *str);
LispObject LispMakeSymbol(char *str);

#endif /* SYMBOLTREE_H */
