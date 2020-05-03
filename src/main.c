/**
 *   \file main.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "config.h"
#include "hal/bsp.h"
#include "lispdoor/functions.h"
#include "lispdoor/gc.h"
#include "lispdoor/memorylayout.h"
#include "lispdoor/objects.h"
#include "lispdoor/print.h"
#include "lispdoor/read.h"

int main(int argc, char *argv[]) {
  LispObject expr;
  stack_bottom = ((char *)&expr) - PROCESS_STACK_SIZE;
  BspInit();
  LispInit();
  /* if (setjmp(toplevel)) { */
  /*   SP = 0; */
  /*   fprintf(stderr, "\n"); */
  /*   if (infile) { */
  /*     fprintf(stderr, "error loading file \"%s\"\n", infile); */
  /*     infile = NULL; */
  /*   } */
  /*   goto repl; */
  /* } */
  printf("LispDoor Version: %s\n", VERSION_STRING);
  printf("%u live objects occupy %u/%u bytes.\n\n",
         *LispNumberOfObjectsAllocated(),
         (LispFixNum)curr_heap - (LispFixNum)from_space, HEAP_SIZE);
  setjmp(LispEnv()->top_level);

  while (1) {
    printf("> ");
    expr = ReadSexpr();
    expr = TopLevelEval(expr);
    if (feof(stdin)) {
      break;
    }
    LispPrintObject(expr, 0);
    /* printf("****\n"); */
    // print2DUtil((struct LispSymbol *)LispEnv()->symbol_table, 0);
    /* printf("----\n"); */
    /* LispPrint(v); */
    /* set(symbol("that"), v); */
    printf("\n\n");
  }
  return 0;
}
