/**
 *   \file memorylayout.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef MEMORYLAYOUT_H
#define MEMORYLAYOUT_H

#include "objects.h"

/* Lisp memory model */
#define ALIGN_BITS (LispIndex)4
#define SYMBOL_TABLE_SIZE (LispIndex)100000
#define PROCESS_STACK_SIZE 512
#define N_STACK 1024
#define HEAP_SIZE (LispFixNum)64 * 1024 /* bytes */

/* 1. symbol table */
extern Byte symbol_table_pool[SYMBOL_TABLE_SIZE];
extern Byte *symbol_table_pool_here;
/* 2. stack */
extern char *stack_bottom;
extern LispObject stack[N_STACK];
extern LispFixNum stack_ptr;
#define PUSH(v)                     \
  do {                              \
    if (stack_ptr > N_STACK) {      \
      stack_ptr = 0;                \
      LispError("stack overflow."); \
    } else {                        \
      stack[stack_ptr++] = (v);     \
    }                               \
  } while (0)
#define POP() (stack[--stack_ptr])
#define POPN(n) (stack_ptr -= (n))
/* 3. GC object space */
extern Byte *from_space;
extern Byte *to_space;
extern Byte *curr_heap;
extern Byte *heap_limit;
extern Byte heap1[HEAP_SIZE];
extern Byte heap2[HEAP_SIZE];
/* 4. read buffer */
extern char buf[256];
/* 5. read character buffer */
#define CHARS_BUF_SIZE 33U
extern int32_t chars_buf[CHARS_BUF_SIZE];
extern LispIndex chars_buf_index_start;
extern LispIndex chars_buf_index_end;
/* 6. gensym name buffer */
#define GEN_SYM_NAME_SIZE 33
extern char gs_name[GEN_SYM_NAME_SIZE];

#endif /* MEMORYLAYOUT_H */
