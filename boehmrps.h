/****************************************************************
 * file boehmrps/boehmrps.h
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Author(s):
 *      Basile Starynkevitch, France   <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****************************************************************/
#ifndef BOEHMRPS_INCLUDED
#define BOEHMRPS_INCLUDED 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <gc.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <unistr.h>
#include <ctype.h>
#include <syslog.h>
#include <limits.h>

#ifndef BPRS_SHORTGIT
#error compile commnd should define BPRS_SHORTGIT
#endif

typedef enum bprs_kind_en Bprs_Kind;
enum bprs_kind_en
{
  BprsKind__None,
  BprsKind_Int,
  BprsKind_String,
  BprsKind_Double,
  BprsKind_Tuple,		/* constant immutable tuple of values */
  BprsKind_Set,			/* constant ordered set of objects */
  BprsKind_Object		/* mutable object */
};
typedef struct bprs_int_st Bprs_Int;
typedef struct bprs_double_st Bprs_Double;
typedef struct bprs_string_st Bprs_String;
typedef struct bprs_tuple_st Bprs_Tuple;
typedef struct bprs_object_st Bprs_Object;


//// common macros
#define BPRS_WARNING(Fmt,...) bprs_warning_at(__FILE__,__LINE__,__func__,\
					      (Fmt),##__VA_ARGS__)
#define BPRS_FATAL(Fmt,...) bprs_fatal_at(__FILE__,__LINE__,__func__,\
					  (Fmt), ##__VA_ARGS__)

/// convention: our human written functions are named brps_*

//// functions in main.c:::::
extern int brps_argc (void);
extern const char *brps_progargv (int rk);
extern const char *bprs_shortgit_main (void);
extern void *bprs_dlsym (const char *name);
extern void bprs_warning_at (const char *srcfil, int lin, const char *fun,
			     const char *fmt, ...)
  __attribute__((format (printf, 4, 5)));
extern void bprs_fatal_at (const char *srcfil, int lin, const char *fun,
			   const char *fmt, ...)
  __attribute__((format (printf, 4, 5), noreturn));
extern void *bprs_program_dlhandle (void);
//// end of functions in main.c




///// functions in util.c
// hash an arbitrary UTF8 string
extern uint32_t bprs_hash_c_string (const char *s);

/// the pok pointer may be null and is set to true on success
extern const char *bprs_shortgit_util (void);
extern Bprs_Int *bprs_make_int (int64_t i);
extern int64_t bprs_get_int (const Bprs_Int * p, bool *pok);
extern uint32_t bprs_hash_int (const Bprs_Int * p, bool *pok);
extern Bprs_Double *bprs_make_double (double d);
extern double bprs_get_double (const Bprs_Double * p, bool *pok);
extern uint32_t bprs_hash_double (const Bprs_Double * p, bool *pok);
extern Bprs_String *bprs_make_string (const char *);
extern const char *bprs_get_string (const Bprs_String * p, bool *pok);
extern uint32_t bprs_hash_string (const Bprs_String * p, bool *pok);
extern uint32_t bprs_byte_len_string (const Bprs_String * ps, bool *pok);
extern uint32_t bprs_utf_len_string (const Bprs_String * ps, bool *pok);

//// in compo.c
extern const char *bprs_shortgit_compo (void);
extern Bprs_Tuple *bprs_tuple_make (void **arr, unsigned siz);
#warning missing bprs_object_make
#endif /*BOEHMRPS_INCLUDED */
