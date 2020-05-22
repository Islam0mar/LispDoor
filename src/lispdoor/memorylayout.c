/*
 *    \file memorylayout.c
 *
 * Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 * This file is part of LispDoor.
 *
 *     LispDoor is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     LispDoor is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with LispDoor.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyrights and
 * permission notices:
 *
 *    Copyright (c) 2008 Jeff Bezanson
 *
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *
 *        * Redistributions of source code must retain the above copyright notice,
 *          this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright notice,
 *          this list of conditions and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution.
 *        * Neither the author nor the names of any contributors may be used to
 *          endorse or promote products derived from this software without specific
 *          prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *    Copyright (c) 1984, Taiichi Yuasa and Masami Hagiya.
 *    Copyright (c) 1990, Giuseppe Attardi.
 *    Copyright (c) 2001, Juan Jose Garcia Ripoll.
 *
 *    ECL is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
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
