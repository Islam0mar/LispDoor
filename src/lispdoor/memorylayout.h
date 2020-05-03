/**
 *   \file memorylayout.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_MEMORYLAYOUT_H_INCLUDED
#define LISPDOOR_MEMORYLAYOUT_H_INCLUDED

#include "lispdoor/objects.h"

/* Lisp memory model */
#define ALIGN_BITS (LispIndex)4
#define SYMBOL_TABLE_SIZE (LispIndex)100
#define PROCESS_STACK_SIZE 50U
#define N_STACK 512U
#define HEAP_SIZE (uint32_t)1 * 1024 /* bytes */
#define TIB_SIZE \
  128U /* Should be divisable by 2 (see HAL_UART_RxCpltCallback)*/
#define SCRATCH_PAD_SIZE 65U

/* 1. symbol table */
extern Byte symbol_table_pool[SYMBOL_TABLE_SIZE];
extern Byte *symbol_table_pool_here;
/* 2. stack */
extern char *stack_bottom;
extern LispObject stack[N_STACK];
extern LispIndex stack_index;
#define PUSH(v)                             \
  do {                                      \
    if (stack_index >= N_STACK) {           \
      stack_index = 0;                      \
      LispError("error: stack overflow\n"); \
    } else {                                \
      stack[stack_index++] = (v);           \
    }                                       \
  } while (0)
#define POP() (stack[--stack_index])
#define POPN(n) (stack_index -= (n))
/* 3. GC object space */
extern Byte *from_space;
extern Byte *to_space;
extern Byte *curr_heap;
extern Byte *heap_limit;
extern Byte heap1[HEAP_SIZE];
extern Byte heap2[HEAP_SIZE];
/* 4. Terminal input buffer FIFO cycle */
extern char terminal_buffer[TIB_SIZE];
extern uint8_t terminal_buffer_get_index;
extern uint8_t terminal_buffer_insert_index;
/* 6. scratch pad buffer */
extern int8_t scratch_pad[SCRATCH_PAD_SIZE];
/* 7. number base to print */
extern uint8_t lisp_number_base;
/* 8. fractional part precision to print */
extern uint8_t lisp_fractional_precision;
/* 9. used for self circular cons*/
extern LispObject cons_flags;
extern LabelTable print_conses;
/* 9. used for reading labels */
extern ReadState *read_state;

#endif /* LISPDOOR_MEMORYLAYOUT_H_INCLUDED */
