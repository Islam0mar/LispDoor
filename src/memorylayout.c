/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "memorylayout.h"

/* Lisp memory model */
/* 1. symbol table */
Byte symbol_table_pool[SYMBOL_TABLE_SIZE];
Byte *symbol_table_pool_here = symbol_table_pool;
/* 2. stack */
char *stack_bottom;
LispObject stack[N_STACK];
LispFixNum stack_ptr = 0;
/* 3. GC object space */
Byte *from_space;
Byte *to_space;
Byte *curr_heap;
Byte *heap_limit;
Byte heap1[HEAP_SIZE];
Byte heap2[HEAP_SIZE];

