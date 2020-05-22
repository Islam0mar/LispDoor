/*
 *    \file STM32F103C8T6.ld
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
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *    Copyright (C) Quantum Leaps, LLC. All rights reserved.
 *
 *    This program is open source software: you can redistribute it and/or
 *    modify it under the terms of the following MIT License (MIT).
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a
 *    copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *    DEALINGS IN THE SOFTWARE.
 *
 *    Contact information:
 *    http://www.state-machine.com
 *    mailto:info@state-machine.com
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 64K
    RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 20K
}
/* The size of the stack used by the application. NOTE: you need to adjust  */
STACK_SIZE = 1024;

/* The size of the heap used by the application. NOTE: you need to adjust   */
HEAP_SIZE = 0;

SECTIONS
{
   .isr_vector : {        /* the vector table */
        KEEP(*(.isr_vector)) /* vector table */
        . = ALIGN(4);
    } >FLASH
    .text : {              /* code and constants */
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
    } >FLASH

    .preinit_array : {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array : {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array : {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(.fini_array*))
        KEEP (*(SORT(.fini_array.*)))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

  /* .fill : { */
	/* 	_code = .; */
  /*   	FILL(0xFFFF); */
  /*   	. = ORIGIN(FLASH) + LENGTH(FLASH) - 1; */
  /*   	BYTE(0xFF) */
  /* 	} >FLASH */
  
     _etext = .;            /* global symbols at end of code */
     __flash_end__ = .;            /* global symbols at end of code */

    .stack : {
        __stack_start__ = .;
        . = . + STACK_SIZE;
        . = ALIGN(4);
        __stack_end__ = .;
    } >RAM

    .data :  AT (_etext) {
        __data_load = LOADADDR (.data);
        __data_start = .;
        *(.my_vector)
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        . = ALIGN(4);
        __data_end__ = .;
        _edata = __data_end__;
    } >RAM

    .bss : {
        __bss_start__ = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = .;
    } >RAM

    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    PROVIDE ( __end__ = _ebss );

    .heap : {
        __heap_start__ = .;
        . = . + HEAP_SIZE;
        . = ALIGN(4);
        __heap_end__ = .;
    } >RAM

    __ram_end__ = .;

    /* Remove information from the standard libraries*/
    /DISCARD/ : {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
