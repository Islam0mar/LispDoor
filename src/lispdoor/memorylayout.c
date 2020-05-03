/**
 *   \file memorylayout.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "lispdoor/memorylayout.h"

/* Lisp memory model */
/* 1. memory pool */
Byte *from_space;
Byte *to_space;
Byte *curr_heap;
Byte *heap_limit;
Byte heap1[HEAP_SIZE];
Byte heap2[HEAP_SIZE];
/* 2. stack */
Byte *stack_bottom;
LispObject stack[N_STACK];
LispIndex stack_index = 0;
/* 3. Terminal input buffer FIFO cycle */
Byte terminal_buffer[TIB_SIZE];
Byte terminal_buffer_get_index;
Byte terminal_buffer_insert_index;
/* 4. scratch pad buffer */
Byte scratch_pad[SCRATCH_PAD_SIZE];
/* 5. number base to print */
Byte lisp_number_base = 10;
/* 6. fractional part precision to print */
Byte lisp_fractional_precision = 10;
/* 7. used for self circular cons*/
LispObject cons_flags;
LabelTable print_conses;
/* 8. used for reading labels */
ReadState *read_state = NULL;
