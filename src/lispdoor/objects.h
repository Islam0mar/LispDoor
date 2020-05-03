/**
 *   \file objects.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef LISPDOOR_OBJECTS_H_INCLUDED
#define LISPDOOR_OBJECTS_H_INCLUDED

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
        Definition of the type of LISP objects.
*/
typedef intptr_t LispFixNum;
typedef uint16_t LispIndex;
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

LispEnvPtr LispEnv();

/* jmp_buf env; */
/* uint8_t state = 0; */
/* extern uint32_t __ram_end__; */
/* extern uint32_t __flash_end__; */
/* FORTH_ASSERT(__ram_end__ == __flash_end__, "shit"); */
#define LISP_NIL ((LispObject)kList)
#define LISP_T ((LispObject)(kCharacter)) /* '\0' char isn't supported */
#define LISP_UNBOUND (OBJ_NULL)
#define LISP_UNBOUNDP(x) (x == LISP_UNBOUND)
#define LISP_NULL(x) ((x) == LISP_NIL)

typedef enum {
  kStart = 0,
  kList = 1,
  kCharacter = 2, /* immediate character */
  kFixNum = 3,    /* immediate fixnum */
  kSingleFloat,
  kDoubleFloat,
  kLongFloat,
  kLastNumber = kLongFloat,
  kSymbol,
  kBitVector,
  kString,
  kCFunction, /* internal */
  kForwarded, /* immediate forwarded flag used by GC */
  kGenSym,    /* internal only, no valid lisp object should have this type */
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

/* Immediate characters:        */
#define LISP_CHARACTER_TAG kCharacter
#define LISP_CharacterP(o) (LISP_IMMEDIATE(o) == kCharacter)
#define LISP_MAKE_CHARACTER(c) \
  ((LispObject)(((LispFixNum)(c << 2) | LISP_CHARACTER_TAG)))
#define LISP_CHAR_CODE(obje) (((LispFixNum)(obje)) >> 2)

#define LISP_NumberP(x) \
  (LISP_TYPE_OF(x) >= kFixNum && LISP_TYPE_OF(x) <= kLastNumber)
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

#define LISP_SYMBOL_TYPE_TAG (kSymTag)
#define LISP_SymbolP(x) ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kSymbol))
#define LISP_GenSymP(x) (((LISP_IMMEDIATE(x) == 0) && ((x)->d.t == kGenSym)))
#define LISP_SYMBOL_GENSYMP(sym) (sym->symbol.stype == kSymGenSym)
#define LISP_SYMBOL_CONSTANTP(sym) (sym->symbol.stype == kSymConstant)

#define LISP_FORWARDEDP(x) ((LISP_IMMEDIATE(x) == 0) && ((x)->d.t ==  kForwarded)))
#define LISP_SET_FORWARDED(obj_old, obj_new) \
  obj_old->d.t = kForwarded;                 \
  obj_old->d.o_new = obj_new;
#define LISP_FORWARD(obj) (obj->d.o_new)

#define LISP_CFunctionP(x) ((LISP_IMMEDIATE(x) == 0) && (x)->d.t == kCFunction)
#define LISP_CFUNCTION_SPECIALP(x) ((x)->cfun.f_type == kFunctionSpecial)

#define LISP_PTR_CONS(x) (LispObject)((intptr_t)(x) + kList)
#define LISP_CONS_PTR(x) ((struct LispCons *)((intptr_t)(x)-kList))
#define LISP_CONS_CAR(x) (*((LispObject *)((intptr_t)(x)-kList)))
#define LISP_CONS_CAR_SAFE(x) (ToCons((x), "car")->car)
#define LISP_CONS_CDR(x) \
  (*((LispObject *)((uintptr_t)x + sizeof(LispObject) - kList)))
#define LISP_CONS_CDR_SAFE(x) (ToCons((x), "cdr")->cdr)
#define LISP_RPLACA(x, v) (LISP_CONS_CAR(x) = (v))
#define LISP_RPLACD(x, v) (LISP_CONS_CDR(x) = (v))

#define LISP_SET_FUNCTION(f_name, f) \
  o = LispMakeSymbol(f_name);        \
  o->symbol.value = LispMakeCFunction(o->symbol.name, f)
#define LISP_SET_CONSTANT_FUNCTION(f_name, f) \
  o = LispMakeSymbol(f_name);                 \
  o->symbol.stype = kSymConstant;             \
  o->symbol.value = LispMakeCFunction(o->symbol.name, f)
#define LISP_SET_VALUE(f_name, v) \
  o = LispMakeSymbol(f_name);     \
  o->symbol.stype = kSymConstant; \
  o->symbol.value = v
#define LISP_SET_CONSTANT_VALUE(f_name, v) \
  o = LispMakeSymbol(f_name);              \
  o->symbol.stype = kSymConstant;          \
  o->symbol.value = v
#define LISP_SET_SPECIAL(f_name, f) \
  o = LispMakeSymbol(f_name);       \
  o->symbol.value = LispMakeCFunctionSpecial(o->symbol.name, f)
#define LISP_SET_CONSTANT_SPECIAL(f_name, f) \
  o = LispMakeSymbol(f_name);                \
  o->symbol.stype = kSymConstant;            \
  o->symbol.value = LispMakeCFunctionSpecial(o->symbol.name, f)

#define LISP_TYPE_OF(o) \
  ((LispType)(LISP_IMMEDIATE(o) ? LISP_IMMEDIATE(o) : ((o)->d.t)))

#define _LISP_HDR uint8_t t
#define _LISP_HDR1(field) uint8_t t, field
#define _LISP_HDR2(field1, field2) uint8_t t, field1, field2

struct LispSingleFloat {
  _LISP_HDR;
  float value; /*  singlefloat value  */
#if ((UINT_MAX) != 0xffffffffu)
  uint32_t padding; /* for 64 bit system */
  /* as we use the content after type */
  /* for storing forward pointer */
#endif
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

enum LispSymType { kSymOrdinary = 0, kSymGenSym, kSymConstant };

struct LispSymbol {
  _LISP_HDR1(stype); /*symbol type */
  LispObject value;
  /* h upper-bound for n = 1000 is less than 15 */
  int8_t height; /* h for AVL tree */
  /* AVL Tree */
  struct LispSymbol *left;
  struct LispSymbol *right;
  /* LispIndex binding;       /\*  index into the bindings array  *\/ */
  char name[1];
};

enum LispCFunctionType { kFunctionOrdinary = 0, kFunctionSpecial };

struct LispCFunction {
  _LISP_HDR1(f_type);
  LispFunc f;
  char *name;
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
  uint32_t self[1];    /*  pointer to the vector  */
};

struct LispString {     /*  vector header  */
  _LISP_HDR;            /*  array element type*/
  LispIndex size;       /*  dimension  */
  LispBaseChar self[1]; /*  pointer to the string  */
};

struct LispGenSym {
  _LISP_HDR1(stype);
  LispIndex id;
};

/*
        dummy type
*/
struct LispDummy {
  _LISP_HDR;
  LispObject o_new;
};

// safe cast operators --------------------------------------------------------
#define IDENTITY(x) (x)
#define SAFECAST_OP_HEADER(l_type, c_type, lisp_to_c_fun) \
  c_type To##l_type(LispObject v, char *fname)
SAFECAST_OP_HEADER(Cons, struct LispCons *, LISP_CONS_PTR);
SAFECAST_OP_HEADER(Symbol, struct LispSymbol *, IDENTITY);
SAFECAST_OP_HEADER(FixNum, LispFixNum, LISP_FIXNUM);
SAFECAST_OP_HEADER(Character, LispIndex, LISP_CHAR_CODE);
SAFECAST_OP_HEADER(CFunction, struct LispCFunction *, IDENTITY);
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
  struct LispCFunction cfun;           /*  c-function  */
  struct LispDummy d;                  /*  dummy  */
};

typedef struct LispDummy LispSmallestStruct;
#define MAKE_FUNC_HEADER(name, union_t, c_type) \
  LispObject LispMake##name(c_type val)

MAKE_FUNC_HEADER(SingleFloat, single_float, float);
MAKE_FUNC_HEADER(DoubleFloat, double_float, double);
MAKE_FUNC_HEADER(LongFloat, long_float, long double);

/* cfunction */
LispObject LispMakeCFunction(char *name, LispFunc fun);
LispObject LispMakeCFunctionSpecial(char *name, LispFunc fun);

/* string */
LispObject LispMakeString(char *str);

/* bit-vector */
LispObject LispBitVectorResize(LispObject bv, LispIndex n);
LispObject LispMakeBitVector(LispIndex n);
LispObject LispMakeBitVectorExactSize(LispIndex n);
LispObject LispMakeInitializedBitVector(LispIndex n, int val);
void LispBitVectorSet(LispObject o, uint32_t n, uint32_t c);
uint32_t LispBitVectorGet(LispObject o, uint32_t n);

/* vector */
LispObject LispMakeVector(LispIndex size);
LispObject LispVectorResize(LispObject v, LispIndex alloc_size);
LispObject LispVectorPush(LispObject v, LispObject value);
LispObject LispVectorPop(LispObject v);

/* gen-symbol */
LispObject LdMakeGenSym(LispIndex nargs);
char *LispSymbolName(LispObject sym);

/* conses */
LispObject MakeCons(void);
LispObject cons_(LispObject car, LispObject cdr);
LispObject cons(LispObject car, LispObject cdr);

/* allocate n consecutive conses */
LispObject ConsReserve(LispIndex n);

/* used for labels */
typedef struct {
  LispIndex n, maxsize;
  LispObject *items;
} LabelTable;

typedef struct _ReadState {
  LabelTable labels;
  LabelTable exprs;
  struct _ReadState *prev;
} ReadState;

#endif /* LISPDOOR_OBJECTS_H_INCLUDED */
