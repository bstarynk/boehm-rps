/****************************************************************
 * file boehm-rps/compo.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description: This file is part of the Reflective Persistent System
 *      (Boehm variant), with functions for composite values
 *
 * Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
 *      <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****/
#include "boehmrps.h"

static const char _bprs_compo_shortgit[] = BPRS_SHORTGIT;

typedef void bprs_finalizer_sig_t (void *ad);

static void bprs_gc_finalizer (void *obj, void *data);

static void *
bprs_malloc_finalized (size_t sz, bprs_finalizer_sig_t *fun)
{
  void *p = GC_MALLOC (sz);
  if (!fun)
    return p;
  void *finaldata = NULL;
  GC_REGISTER_FINALIZER (p, bprs_gc_finalizer, finaldata, NULL, NULL);
#warning bprs_malloc_finalized need to be reviewed
   /*** should use GC_REGISTER_FINALIZER(p,gcfinalizer,
	clientdatd,gcfinalized*old,od)
    ***/
  BPRS_FATAL ("unimplemented bprs_malloc_finalized sz=%ld", (long) sz);
  return p;
}

void
bprs_gc_finalizer (void *obj, void *data)
{
  BPRS_FATAL ("unimplemented bprs_gc_finalizer obj=%p data=%p", obj, data);
}

const char *
bprs_shortgit_compo (void)
{
  return _bprs_compo_shortgit;
}				/* end bprs_shortgit_compo */

#define BPRS_TUPLE_MAGIC 39736559	/* 0x25e54ef */
#define BPRS_MASK_TUPLE 0xf7fe
struct bprs_tuple_st
{
  Bprs_Kind tu_kind;
  int16_t tu_gap;
  int32_t tu_magic;
  uint32_t tu_hash;
  uint32_t tu_size;
  void *tu_val[];
};


Bprs_Tuple *
bprs_tuple_make_arr (void **arr, unsigned siz)
{
  if (!arr || (uintptr_t) arr < 0x1000)
    return NULL;
  if (siz == 0)
    return NULL;
  if (siz > INT_MAX / 8)
    BPRS_FATAL ("too many components %ud in tuple", siz);
  Bprs_Tuple *tup = GC_MALLOC (sizeof (Bprs_Tuple) + siz * sizeof (void *));
  memset ((void *) tup, 0, (sizeof (Bprs_Tuple) + siz * sizeof (void *)));
  uint32_t h = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      Bprs_Kind k = bprs_value_kind (arr[ix]);
      if (k != BprsKind__None)
	{
	  uint32_t hcomp = bprs_value_hash (arr[ix]);
	  if (hcomp != 0)
	    {
	      tup->tu_val[ix] = arr[ix];
	      h = (h * 673) ^ (ix + hcomp * 691);
	    };
	}
      else
	tup->tu_val[ix] = NULL;
    };
  if (h == 0)
    h = 3 + (siz & 0xff);
  tup->tu_kind = BprsKind_Tuple;
  tup->tu_gap = (intptr_t) tup & BPRS_MASK_TUPLE;
  tup->tu_size = siz;
  tup->tu_hash = h;
  tup->tu_magic = BPRS_TUPLE_MAGIC;
  return tup;
}				/* end bprs_tuple_make_arr */


Bprs_Tuple *
bprs_tuple_make_args (unsigned siz, ...)
{
  Bprs_Tuple *res = NULL;
  va_list args;
  const int small_len = 64;
  const int max_len = 1024;
  void *small_arr[small_len];
  if (siz == 0)
    return NULL;
  if (siz < small_len)
    {
      memset (small_arr, 0, sizeof (small_arr));
      va_start (args, siz);
      for (unsigned ix = 0; ix < siz; ix++)
	{
	  const void *curarg = va_arg (args, const void *);
	  if (bprs_value_kind (curarg) == BprsKind__None)
	    continue;
	  small_arr[ix] = curarg;
	}
      va_end (args);
      return bprs_tuple_make_arr (small_arr, siz);
    };
  if (siz > max_len)
    BPRS_FATAL ("bprs_tuple_make_arg called with too many arguments %d (>%d)",
		(int) siz, max_len);
  void **arrptr = GC_MALLOC ((siz + 1) * sizeof (void *));
  memset (arrptr, 0, (siz + 1) * sizeof (void *));
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      const void *curarg = va_arg (args, const void *);
      if (bprs_value_kind (curarg) == BprsKind__None)
	continue;
      arrptr[ix] = curarg;
    }
  va_end (args);
  res = bprs_tuple_make_arr (arrptr, siz);
  memset (arrptr, 0, siz * sizeof (void *));
  GC_FREE (arrptr);
  return res;
}				/* end bprs_tuple_make_args */


unsigned
bprs_tuple_length (const Bprs_Tuple *tup)
{

  if (!tup || (uintptr_t) tup < 0x1000
      || tup->tu_kind != BprsKind_Tuple
      || tup->tu_magic != BPRS_TUPLE_MAGIC
      || tup->tu_gap != ((intptr_t) tup & BPRS_MASK_TUPLE))
    return 0;
  return tup->tu_size;
}				/* end bprs_tuple_length */

void *
bprs_tuple_nth (const Bprs_Tuple *tup, int rk)
{
  unsigned ln = bprs_tuple_length (tup);
  if (ln == 0)
    return NULL;
  if (rk < 0)
    rk += ln;
  if (rk > 0 && rk < (int) ln
      && bprs_value_kind (tup->tu_val[rk]) != BprsKind__None)
    return tup->tu_val[rk];
  return NULL;
}				/* end bprs_tuple_nth */

uint32_t
bprs_hash_tuple (const Bprs_Tuple *tup, bool *pok)
{
  if (!tup || (uintptr_t) tup < 0x1000
      || tup->tu_kind != BprsKind_Tuple
      || tup->tu_magic != BPRS_TUPLE_MAGIC
      || tup->tu_gap != ((intptr_t) tup & BPRS_MASK_TUPLE))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  assert (tup->tu_hash != 0);
  if (pok)
    *pok = true;
  return tup->tu_hash;
}				/* end bprs_tuple_hash */

struct bprs_object_st
{
  Bprs_Kind ob_kind;
  int16_t ob_gap;
  int32_t ob_magic;
  uint32_t ob_hash;
  uint32_t ob_size;
  uint32_t ob_hi;
  uint64_t ob_lo;
  pthread_mutex_t ob_mtx;
  void *ob_val[];
#warning missing fields (eg routine pointers and private data) in object
};

/* end boehm-rps/compo.c */
