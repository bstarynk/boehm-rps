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
#define _GNU_SOURCE 1
#include <stdio.h>
#include <gc.h>
#include <gc/gc_pthread_redirects.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <unistr.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <limits.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <readline/readline.h>

#ifndef BPRS_SHORTGIT
#error compile commnd should define BPRS_SHORTGIT
#endif

#define BPRS_MAGIC_STRING "Boehm-Rps-Store"

enum bprs_kind_en
{
  BprsKind__None,
  BprsKind_Int,
  BprsKind_String,
  BprsKind_Double,
  BprsKind_Tuple,		/* constant immutable tuple of values */
  BprsKind_Set,			/* constant ordered set of objects */
  BprsKind_Object,		/* mutable object */
  BprsKind__Last
};
typedef enum bprs_kind_en Bprs_Kind;

struct bprs_value_st
{
  const Bprs_Kind v_kind;
  const int16_t v_gap;
  const int32_t v_magic;
  const char v_bytes[];
};
typedef struct bprs_value_st Bprs_Value;
typedef struct bprs_int_st Bprs_Int;
typedef struct bprs_double_st Bprs_Double;
typedef struct bprs_string_st Bprs_String;
typedef struct bprs_tuple_st Bprs_Tuple;
typedef struct bprs_set_st Bprs_Set;
typedef struct bprs_object_st Bprs_Object;

#define BPRS_INT_MAGIC 68140917	/* 0x40fbf75 */
#define BPRS_MASK_INT 0x7fef
struct bprs_int_st
{
  Bprs_Kind i_kind;
  int16_t i_gap;
  int32_t i_magic;
  int64_t i_val;
};


#define BPRS_DOUBLE_MAGIC 171454729	/* 0xa383109 */
#define BPRS_MASK_DOUBLE 0xffff
struct bprs_double_st
{
  Bprs_Kind d_kind;
  int16_t d_gap;
  int32_t d_magic;
  double d_val;
};


#define BPRS_STRING_MAGIC 654689243	/* 0x2705c3db */
#define BPRS_MASK_STRING 0x3ffd
struct bprs_string_st
{
  Bprs_Kind s_kind;
  int16_t s_gap;
  int32_t s_magic;
  uint32_t s_hash;
  uint32_t s_bytlen;
  uint32_t s_utflen;
  char s_val[];
};

#define BPRS_TUPLE_MAGIC 39736559	/* 0x25e54ef */
#define BPRS_MASK_TUPLE 0xf7fe
struct bprs_tuple_st
{
  Bprs_Kind tu_kind;
  int16_t tu_gap;
  int32_t tu_magic;
  uint32_t tu_hash;
  uint32_t tu_size;
  const void *tu_val[];
};


#define BPRS_SET_MAGIC 80044785	/* 0x4c562f1 */
#define BPRS_MASK_SET 0xfdfe
struct bprs_set_st
{
  Bprs_Kind set_kind;
  uint16_t set_gap;
  uint32_t set_magic;
  uint32_t set_hash;
  uint32_t set_size;
  const Bprs_Object *set_elem[];
};


struct bprs_attrpair_st
{
  Bprs_Object *at_obj;
  void *at_val;
};

struct bprs_object_st
{
  Bprs_Kind ob_kind;
  uint16_t ob_gap;
  uint32_t ob_magic;
  uint32_t ob_index;		/* index in _bprs_objhash_arr */
  uint32_t ob_hash;
  uint32_t ob_nbcomp;
  uint32_t ob_nbattr;
  uint32_t ob_sizcomp;
  uint32_t ob_sizattr;
  uint32_t ob_hi;
  uint64_t ob_lo;
  char ob_prefixrout[8];
  void *ob_routptr;
  pthread_mutex_t ob_mtx;
  struct bprs_attrpair_st *ob_attrs;
  void **ob_comps;
  void *ob_data;
  void *ob_xtra;
};
#define BPRS_OBJHASH_EMPTY (Bprs_Object*)0x7
#define BPRS_OBJECT_MAGIC  943520863	/*0x383cfc5f */
#define BPRS_MASK_OBJECT 0xf5fc
extern void bprs_initialize_objects (void);
extern bool bprs_extract_object_id (const Bprs_Object *pob,
				    uint32_t *phi, uint64_t *plo);

extern bool bprs_verbose;

//// common macros
#define BPRS_WARNING(Fmt,...) bprs_warning_at(__FILE__,__LINE__,__func__,\
                                              (Fmt),##__VA_ARGS__)
#define BPRS_FATAL(Fmt,...) bprs_fatal_at(__FILE__,__LINE__,__func__,\
                                          (Fmt), ##__VA_ARGS__)

//// in generated _list_.c
extern const char bprs_shortgit[];
extern const char bprs_sourcedir[];
extern const char *bprs_human_files[];

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
extern uint32_t bprs_value_hash (const void *);	/* or else 0 if invalid value */
extern const char *bprs_executable (void);	/* give the current executable path */
extern const char *bprs_host (void);	/* give the host name */
extern Bprs_Kind bprs_value_kind (const void *);
//// end of functions in main.c




///// functions in util.c
// hash an arbitrary UTF8 string
extern uint32_t bprs_hash_c_string (const char *s);

/// the pok pointer may be null and is set to true on success
extern const char *bprs_shortgit_util (void);
extern Bprs_Int *bprs_make_int (int64_t i);
extern int64_t bprs_get_int (const Bprs_Int *p, bool *pok);
extern uint32_t bprs_hash_int (const Bprs_Int *p, bool *pok);
extern Bprs_Double *bprs_make_double (double d);
extern double bprs_get_double (const Bprs_Double *p, bool *pok);
extern uint32_t bprs_hash_double (const Bprs_Double *p, bool *pok);
extern Bprs_String *bprs_make_string (const char *);
extern const char *bprs_get_string (const Bprs_String * p, bool *pok);
extern uint32_t bprs_hash_string (const Bprs_String * p, bool *pok);
extern uint32_t bprs_byte_len_string (const Bprs_String * ps, bool *pok);
extern uint32_t bprs_utf_len_string (const Bprs_String * ps, bool *pok);
/// conversions string to/from base 36
extern uint64_t bprs_b36_to_uint64 (const char *s, const char **end,
				    bool *pok);
extern bool bprs_uint64_to_b36 (uint64_t i, char *buf, int bufsiz);

/// checked conversion routines (returns NULL on failure)
extern const Bprs_Value *bprs_boxed_int_as_value (const Bprs_Int *);
extern const Bprs_Int *bprs_value_to_boxed_int (const Bprs_Value *);
extern const Bprs_Value *bprs_boxed_double_as_value (const Bprs_Double *);
extern const Bprs_Double *bprs_value_to_boxed_double (const Bprs_Value *);
extern const Bprs_Value *bprs_boxed_string_as_value (const Bprs_String *);
extern const Bprs_String *bprs_value_to_boxed_string (const Bprs_Value *);




//// in compo.c
extern const char *bprs_shortgit_compo (void);
extern Bprs_Tuple *bprs_tuple_make_arr (const void **arr, unsigned siz);
extern Bprs_Tuple *bprs_tuple_make_args (unsigned siz, ...);
extern const Bprs_Value *bprs_boxed_tuple_as_value (const Bprs_Tuple *tup);
extern const Bprs_Tuple *bprs_value_to_boxed_tuple (const Bprs_Value *val);

extern uint32_t bprs_hash_tuple (const Bprs_Tuple *tup, bool *pok);
extern unsigned bprs_tuple_length (const Bprs_Tuple *rup);
extern const void *bprs_tuple_nth (const Bprs_Tuple *tup, int rk);
extern uint32_t bprs_hash_object (const Bprs_Object *pob, bool *pok);
extern Bprs_Object *bprs_object_find (uint32_t hi, uint64_t lo);
extern Bprs_Object *bprs_object_load (FILE *fil, uint32_t hi, uint64_t lo);
extern Bprs_Object *bprs_object_make (unsigned compsize, unsigned attrsize);
extern bool bprs_object_to_str (Bprs_Object *pob, char *buf, unsigned bufsiz);
extern int bprs_object_cmp (const void *pobleft, const void *pobright);
extern uint32_t bprs_hash_for_object_hi_lo (uint32_t hi, uint64_t lo);

extern Bprs_Set *bprs_set_make_arr (const void **arr, unsigned siz);
extern Bprs_Set *bprs_set_make_args (unsigned siz, ...);
extern uint32_t bprs_hash_set (const Bprs_Set *pset, bool *pok);
extern bool bprs_set_contains (const Bprs_Set *pset,
			       const Bprs_Object *pobelem);
extern unsigned bprs_set_cardinal (const Bprs_Set *pset, bool *pok);
extern Bprs_Object *bprs_set_nth_elem (const Bprs_Set *pset, int n);

//// in load.c
typedef struct bprs_loaded_file_st Bprs_Loaded_File;
extern const char *bprs_shortgit_load (void);

/* if path starts with ! or | use popen */
extern Bprs_Loaded_File *bprs_open_load_file (const char *path);
extern void bprs_process_loaded_file (Bprs_Loaded_File *);



/****************
 **                           for GNU Emacs...
 ** Local Variables: ;;
 ** compile-command: "make -j2" ;;
 ** End: ;;
 ****************/

#endif /*BOEHMRPS_INCLUDED */
