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

/* end boehm-rps/util.c */
