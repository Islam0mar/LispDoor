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

int main() {
  LispObject expr;
  stack_bottom = ((Byte *)&expr) - PROCESS_STACK_SIZE;
  BspInit();
  LispInit();
  LispPrintStr("LispDoor Version: " VERSION_STRING "\n");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        *LispNumberOfObjectsAllocated(), 10));
  LispPrintStr(" live objects occupy ");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE,
                        (uintptr_t)curr_heap - (uintptr_t)from_space, 10));
  LispPrintStr("/");
  LispPrintStr(Uint2Str((char *)scratch_pad, SCRATCH_PAD_SIZE, HEAP_SIZE, 10));
  LispPrintStr(" bytes.\n\n");
  setjmp(LispEnv()->top_level);

  while (1) {
    LispPrintStr("> ");
    expr = ReadSexpr();
    expr = TopLevelEval(expr);
    LispPrintObject(expr, false);
    LispPrintStr("\n\n");
  }
  return 0;
}
