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
LispIndex stack_index = 0;
/* 3. GC object space */
Byte *from_space;
Byte *to_space;
Byte *curr_heap;
Byte *heap_limit;
Byte heap1[HEAP_SIZE];
Byte heap2[HEAP_SIZE];
/* 4. Terminal input buffer FIFO cycle */
char terminal_buffer[TIB_SIZE];
uint8_t terminal_buffer_get_index;
uint8_t terminal_buffer_insert_index;
/* 6. scratch pad buffer */
int8_t scratch_pad[SCRATCH_PAD_SIZE];
/* 7. number base to print */
uint8_t lisp_number_base = 10;
/* 8. fractional part precision to print */
uint8_t lisp_fractional_precision = 10;
/* 9. used for self circular cons*/
LispObject cons_flags;
LabelTable print_conses;
/* 9. used for reading labels */
ReadState *read_state = NULL;
