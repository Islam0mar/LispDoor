/**
 *   \file gc.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef GC_H
#define GC_H

#include "objects.h"

/* Data allocation */
void GC();
void *GcMalloc(LispIndex num_of_bytes);
LispObject LispAllocObject(LispType t, LispIndex extra_size);
void *SymMalloc(LispIndex num_of_bytes);
LispObject GcRrelocate(LispObject v);
void TraceGlobals();

#endif /* GC_H */
