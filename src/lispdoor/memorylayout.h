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
extern int STACK_SIZE; /* defined in linker script */
#define PROCESS_STACK_SIZE STACK_SIZE
#define N_STACK 512U
#define HEAP_SIZE (LispIndex)(7 * 1024+ 376) /* bytes */
#define TIB_SIZE \
  256U /* Should be divisable by 2 (see HAL_UART_RxCpltCallback)*/
#define SCRATCH_PAD_SIZE 128U

/* 1. memory pool */
extern Byte *from_space;
extern Byte *to_space;
extern Byte *curr_heap;
extern Byte *heap_limit;
extern Byte heap1[HEAP_SIZE];
extern Byte heap2[HEAP_SIZE];
/* 2. stack */
extern Byte *stack_bottom;
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
/* 3. Terminal input buffer FIFO cycle */
extern Byte terminal_buffer[TIB_SIZE];
extern Byte terminal_buffer_get_index;
extern Byte terminal_buffer_insert_index;
/* 4. scratch pad buffer */
extern Byte scratch_pad[SCRATCH_PAD_SIZE];
/* 5. number base to print */
extern Byte lisp_number_base;
/* 6. fractional part precision to print */
extern Byte lisp_fractional_precision;
/* 7. used for self circular cons*/
extern LispObject cons_flags;
extern LabelTable print_conses;
/* 8. used for reading labels */
extern ReadState *read_state;

#endif /* LISPDOOR_MEMORYLAYOUT_H_INCLUDED */
