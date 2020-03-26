/**
 *   \file lispdoor.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef OBJECTS_H
#define OBJECTS_H

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
        Definition of the type of LISP objects.
*/
typedef intptr_t LispFixNum;
typedef uintptr_t LispIndex;
typedef union LispUnion *LispObject;
typedef uint8_t Byte;
typedef char LispBaseChar;
typedef LispObject LispReturn;
typedef LispIndex LispNArg;
typedef LispObject (*LispFunc)(LispNArg narg);

/*
        Object NULL value.
        It should not coincide with any legal object value.
*/
#define OBJ_NULL ((LispObject)NULL)
#define LISP_MULTIPLE_VALUES_LIMIT 64

/*
 * Per-thread data.
 */

typedef struct _LispEnv *LispEnvPtr;
struct _LispEnv {
  /* Array where values are returned by functions. */
  LispIndex nvalues;
  LispObject values[LISP_MULTIPLE_VALUES_LIMIT];

  /* Cons Frame to store lexical scope */
  LispObject frame;

  /* toplevel jmp for errors*/
  jmp_buf top_level;

  /* symbol_table */
  LispObject symbol_table;

  /*   /\* Current stack frame *\/ */
  /* cl_object stack_frame; */

  /* /\* */
  /*  * The lisp stack, which is used mainly for keeping the arguments of a */
  /*  * function before it is invoked, and also by the compiler and by the */
  /*  * reader when they are building some data structure. */
  /*  *\/ */
  /* cl_index stack_size; */
  /* cl_index stack_limit_size; */
  /* cl_object *stack; */
  /* cl_object *stack_top; */
  /* cl_object *stack_limit; */
};

const LispEnvPtr LispEnv();

/* jmp_buf env; */
/* uint8_t state = 0; */
/* extern uint32_t __ram_end__; */
/* extern uint32_t __flash_end__; */
/* FORTH_ASSERT(__ram_end__ == __flash_end__, "shit"); */
#define LISP_NIL ((LispObject)kList)
#define LISP_T ((LispObject)(kBuiltIn))
#define LISP_UNBOUND (OBJ_NULL)
#define LISP_UNBOUNDP(x) (x == LISP_UNBOUND)
#define LISP_NULL(x) ((x) == LISP_NIL)

typedef enum {
  kStart = 0,
  kList = 1,
  kBuiltIn =
      2, /* immediate builtin flag used to correctly print builtin functions */
  kFixNum = 3, /* immediate fixnum */
  kSingleFloat,
  kDoubleFloat,
  kLongFloat,
  kLastNumber = kLongFloat,
  kSymbol,
  kBitVector,
  kString,
  kGenSym,
  kVector
} LispType;

/*
        Definition of each implementation type.
*/

#define LISP_TAG_BITS 2
#define LISP_IMMEDIATE(o) ((LispFixNum)(o)&3)
#define LISP_IMMEDIATE_TAG 3

#define LISP_TO_BOOL(x) ((x) != LISP_NIL)
#define LISP_MAKE_BOOL(x) ((x) ? LISP_T : LISP_NIL)

/* Immediate fixnums:           */
#define LISP_FIXNUM_TAG kFixNum
#define LISP_FixNumP(o) (LISP_IMMEDIATE(o) == kFixNum)
#define LISP_MAKE_FIXNUM(n) ((LispObject)(((LispFixNum)(n) << 2) | kFixNum))
#define LISP_FIXNUM_LOWER(a, b) ((LispFixNum)(a) < (LispFixNum)(b))
#define LISP_FIXNUM_GREATER(a, b) ((LispFixNum)(a) > (LispFixNum)(b))
#define LISP_FIXNUM_LEQ(a, b) ((LispFixNum)(a) <= (LispFixNum)(b))
#define LISP_FIXNUM_GEQ(a, b) ((LispFixNum)(a) >= (LispFixNum)(b))
#define LISP_FIXNUM_PLUSP(a) ((LispFixNum)(a) > (LispFixNum)LISP_make_fixnum(0))
#define LISP_FIXNUM_MINUSP(a) ((LispFixNum)(a) < (LispFixNum)(0))
#define LISP_FIXNUM(a) (((LispFixNum)(a)) >> 2)

/* builtin value */
#define LISP_BUILTIN_TAG kBuiltIn
#define LISP_BuiltInP(o) (LISP_IMMEDIATE(o) == kBuiltIn)
#define LISP_BUILTIN_TO_SYM(o) (LispObject)((intptr_t)(o)-kBuiltIn)
#define LISP_SYM_TO_BUILTIN(o) (LispObject)((intptr_t)(o) + kBuiltIn)

#define LISP_NumberP(t) (t >= kFixNum && t <= kLastNumber)
#define LISP_RealP(t) (t >= kFixNum && t < kComplex)
#define LISP_VectorP(x) ((LISP_IMMEDIATE(x) == 0) && (x)->d.t == kVector)
#define LISP_BitVectorP(x) \
  ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kBitVector))
#define LISP_StringP(x) ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kString))
#define LISP_ExtendedStringP(x) 0
/* TODO: big_nums*/
/* #define LISP_BigNumP(x) ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kbignum))
 */
#define LISP_SingleFloatP(x) \
  ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kSingleFloat))
#define LISP_DoubleFloatP(x) \
  ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kDoubleFloat))
#define LISP_LongFloatP(x) \
  ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kLongFloat))
#define LISP_SINGLE_FLOAT(o) ((o)->single_float.value)
#define LISP_DOUBLE_FLOAT(o) ((o)->double_float.value)
#define LISP_LONG_FLOAT(o) ((o)->long_float.value)

#define LISP_ListP(x) (LISP_IMMEDIATE(x) == kList)
#define LISP_ConsP(x) (LISP_ListP(x) && !LISP_NULL(x))
#define LISP_ATOM(x) (LISP_NULL(x) || !LISP_ListP(x))

#define LISP_SYMBOL_TYPE_TAG (kSymBuiltIn - 1)
#define LISP_SymbolP(x) (((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kSymbol)))
#define LISP_GenSymP(x) (((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kGenSym)))
#define LISP_STRUCT_BUILTINP(sym) (sym->stype & kSymBuiltIn)
#define LISP_SYMBOL_BUILTINP(sym) \
  ((LISP_SymbolP(sym)) && (sym->symbol.stype & kSymBuiltIn))
#define LISP_SYMBOL_SPECIALP(sym) \
  ((LISP_SymbolP(sym)) &&         \
   (sym->symbol.stype & LISP_SYMBOL_TYPE_TAG) == kSymSpecial)
#define LISP_SYMBOL_ORDINARYP(sym) \
  ((LISP_SymbolP(sym)) &&          \
   (sym->symbol.stype & LISP_SYMBOL_TYPE_TAG) == kSymOrdinary)
#define LISP_SYMBOL_CONSTANTP(sym) \
  ((LISP_SymbolP(sym)) && (sym->symbol.stype & kSymConstant))
#define LISP_SYMBOL_GENSYMP(sym) \
  ((LISP_SymbolP(sym)) &&        \
   (sym->symbol.stype & LISP_SYMBOL_TYPE_TAG) == kSymGenSym)

#define LISP_PTR_CONS(x) (LispObject)((intptr_t)(x) + kList)
#define LISP_CONS_PTR(x) ((struct LispCons *)((intptr_t)(x)-kList))
#define LISP_CONS_CAR(x) (*(LispObject *)((intptr_t)x - kList))
#define LISP_CONS_CDR(x) \
  (*(LispObject *)((intptr_t)x + sizeof(LispObject) - kList))
#define LISP_RPLACA(x, v) (LISP_CONS_CAR(x) = (v))
#define LISP_RPLACD(x, v) (LISP_CONS_CDR(x) = (v))

#define LISP_SET_OBJ(s, v) (((LispObject)s)->symbol.value.obj = (v))
#define LISP_SET_CONSTANT(s, v)              \
  (((LispObject)s)->symbol.value.obj = (v)); \
  (((LispObject)s)->symbol.stype |= (kSymConstant | kSymBuiltIn))
#define LISP_SET_FUNC(s, v)                   \
  (((LispObject)s)->symbol.value.func = (v)); \
  (((LispObject)s)->symbol.stype |= kSymBuiltIn)
#define LISP_SET_SPECIAL(s, v)                \
  (((LispObject)s)->symbol.value.func = (v)); \
  (((LispObject)s)->symbol.stype = (kSymSpecial | kSymBuiltIn))
#define LISP_SET_SPECIAL_CONSTANT(s, v)       \
  (((LispObject)s)->symbol.value.func = (v)); \
  (((LispObject)s)->symbol.stype = (kSymConstant | kSymSpecial | kSymBuiltIn))

#define LISP_TYPE_OF(o) \
  ((LispType)(LISP_IMMEDIATE(o) ? LISP_IMMEDIATE(o) : ((o)->d.t)))

/* gen-sym */
extern LispIndex gen_sym_ctr;
#define GEN_SYM_NAME_SIZE 33
char gs_name[GEN_SYM_NAME_SIZE];

#define _LISP_HDR uint8_t t
#define _LISP_HDR1(field) uint8_t t, field
#define _LISP_HDR2(field1, field2) uint8_t t, field1, field2

struct LispSingleFloat {
  _LISP_HDR;
  float value; /*  singlefloat value  */
};

struct LispDoubleFloat {
  _LISP_HDR;
  double value; /*  doublefloat value  */
};

struct LispLongFloat {
  _LISP_HDR;
  long double value; /*  longdoublefloat value  */
};

struct LispCons {
  LispObject car; /*  car  */
  LispObject cdr; /*  cdr  */
};

enum LispSymType { /*  symbol type  */
                   kSymOrdinary = 0,
                   kSymGenSym,
                   kSymSpecial,
                   kSymConstant = 32,
                   kSymBuiltIn = 64
};

struct LispSymbol {
  _LISP_HDR1(stype);
  union {
    /*  global value of the symbol  */
    LispObject obj;
    LispFunc func;
  } value;
  uint8_t height; /*  h for AVL tree ,symbol type */
  /* AVL Tree */
  struct LispSymbol *left;
  struct LispSymbol *right;
  /* LispIndex binding;       /\*  index into the bindings array  *\/ */
  char name[1];
};

struct LispVector {   /*  vector header  */
  _LISP_HDR;          /*  array element type*/
  LispIndex size;     /*  dimension  */
  LispIndex fillp;    /*  fill pointer  */
                      /*  For simple vectors, fillp is equal to dim. */
  LispObject self[1]; /*  pointer to the vector  */
};

struct LispBitVector { /*  vector header  */
  _LISP_HDR;           /*  array element type*/
  LispIndex size;      /*  dimension  */
  uint32_t self[0];    /*  pointer to the vector  */
};

struct LispString {     /*  vector header  */
  _LISP_HDR;            /*  array element type*/
  LispIndex size;       /*  dimension  */
  LispBaseChar self[0]; /*  pointer to the string  */
};

struct LispGenSym {
  _LISP_HDR1(stype);
  union {
    /*  value of the gensym  */
    LispObject obj;
    LispFunc func;
  } value;
  LispIndex id;
};

/*
        dummy type
*/
struct LispDummy {
  _LISP_HDR;
};

// safe cast operators --------------------------------------------------------
#define IDENTITY(x) (x)
#define SAFECAST_OP_HEADER(l_type, c_type, lisp_to_c_fun) \
  c_type To##l_type(LispObject v, char *fname)
SAFECAST_OP_HEADER(Cons, struct LispCons *, LISP_CONS_PTR);
SAFECAST_OP_HEADER(Symbol, struct LispSymbol *, IDENTITY);
SAFECAST_OP_HEADER(FixNum, LispFixNum, LISP_FIXNUM);
SAFECAST_OP_HEADER(BuiltIn, struct LispSymbol *, LISP_BUILTIN_TO_SYM);
SAFECAST_OP_HEADER(Vector, struct LispVector *, IDENTITY);
SAFECAST_OP_HEADER(String, struct LispString *, IDENTITY);
SAFECAST_OP_HEADER(BitVector, struct LispBitVector *, IDENTITY);
SAFECAST_OP_HEADER(SingleFloat, struct LispSingleFloat *, IDENTITY);
SAFECAST_OP_HEADER(DoubleFloat, struct LispDoubleFloat *, IDENTITY);
SAFECAST_OP_HEADER(LongFloat, struct LispLongFloat *, IDENTITY);
SAFECAST_OP_HEADER(GenSym, struct LispGenSym *, IDENTITY);

/*
        Definition of lispunion.
*/
union LispUnion {
  /* TODO: add bignum */
  /* struct LISP_bignum       big;            /\*  bignum  *\/ */
  struct LispSingleFloat single_float; /*  single floating-point number  */
  struct LispDoubleFloat double_float; /*  double floating-point number  */
  struct LispLongFloat long_float;     /*  long-float */
  struct LispSymbol symbol;            /*  symbol  */
  struct LispVector vector;            /*  vector  */
  struct LispString string;            /*  string  */
  struct LispBitVector bit_vector;     /*  bitvector  */
  struct LispGenSym gen_sym;           /*  gensym  */
  struct LispDummy d;                  /*  dummy  */
};

/* string */
LispObject LispMakeString(char *str);

/* bit-vector */
LispObject LispBitVectorResize(LispObject bv, LispIndex n);
LispObject LispMakeBitVector(LispIndex n);
void LispBitVectorSet(LispObject o, uint32_t n, uint32_t c);
uint32_t LispBitVectorGet(LispObject o, uint32_t n);

/* vector */
LispObject LispMakeVector(LispIndex size);
LispObject LispVectorResize(LispObject v, LispIndex alloc_size);
LispObject LispVectorPush(LispObject v, LispObject value);
LispObject LispVectorPop(LispObject v, LispObject value);

/* gen-symbol */
LispObject LispMakeGenSym(LispIndex nargs);
char *LispSymbolName(LispObject sym);

/* conses */
LispObject MakeCons(void);
LispObject cons_(LispObject car, LispObject cdr);
LispObject cons(LispObject car, LispObject cdr);

/* allocate n consecutive conses */
LispObject ConsReserve(LispIndex n);

#endif /* OBJECTS_H */
