/****************************************************************
 * file boehm-rps/main.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description: This file is part of the Reflective Persistent System
 *      (Boehm variant) It contains several utility functions
 *
 * Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
 *      <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****/
#include "boehmrps.h"

static const char _bprs_util_shortgit[] = BPRS_SHORTGIT;

#define BPRS_INT_MAGIC 68140917	/* 0x40fbf75 */
struct bprs_int_st
{
  Bprs_Kind i_kind;
  int16_t i_gap;
  int32_t i_magic;
  int64_t i_val;
};

const char *
bprs_shortgit_util (void)
{
  return _bprs_util_shortgit;
}				/* end bprs_shortgit_util */


Bprs_Int *
bprs_make_int (int64_t i)
{
  Bprs_Int *p = GC_MALLOC_ATOMIC (sizeof (Bprs_Int));
  p->i_kind = BprsKind_Int;
  p->i_gap = (intptr_t) p & 0xffff;
  p->i_magic = BPRS_INT_MAGIC;
  p->i_val = i;
  return p;
}				/* end bprs_make_int */

int64_t
bprs_get_int (const Bprs_Int *p, bool *pok)
{
  if (!p || p->i_kind != BprsKind_Int || p->i_magic != BPRS_INT_MAGIC
      || p->i_gap != ((intptr_t) p & 0xffff))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  return p->i_val;
}				/* end bprs_get_int */
