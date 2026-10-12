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
bprs_tuple_make (void **arr, unsigned siz)
{
  if (!arr || (uintptr_t) arr < 0x1000)
    return NULL;
  if (siz > INT_MAX / 8)
    BPRS_FATAL ("too many components %ud in tuple", siz);
  Bprs_Tuple *tup = GC_MALLOC (sizeof (Bprs_Tuple) + siz * sizeof (void *));
  tup->tu_kind = BprsKind_Tuple;
#warning incomplete bprs_tuple_make
  BPRS_FATAL ("unimplemented bprs_tuple_make tup@%p", tup);
  return tup;
}				/* end bprs_tuple_make */

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

/* end boehm-rps/util.c */
