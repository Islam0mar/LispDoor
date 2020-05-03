/**
 *   \file gc.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_GC_H_INCLUDED
#define LISPDOOR_GC_H_INCLUDED

#include "lispdoor/objects.h"

/* Data allocation */
void GC();
void *GcMalloc(LispIndex num_of_bytes);
LispObject LispAllocObject(LispType t, LispIndex extra_size);
void *SymMalloc(LispIndex num_of_bytes);
LispIndex *LispNumberOfObjectsAllocated();

#endif /* LISPDOOR_GC_H_INCLUDED */
