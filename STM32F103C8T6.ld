/*
*****************************************************************************
**

**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32F103C8Tx Device with
**                64KByte FLASH, 20KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
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
    .isr_vector : {        /* the vector table goes FIRST into FLASH */
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
