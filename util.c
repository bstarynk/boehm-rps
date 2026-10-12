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
  p->i_gap = (intptr_t) p & BPRS_MASK_INT;
  p->i_magic = BPRS_INT_MAGIC;
  p->i_val = i;
  return p;
}				/* end bprs_make_int */

int64_t
bprs_get_int (const Bprs_Int *p, bool *pok)
{
  if (!p || p->i_kind != BprsKind_Int
      || p->i_magic != BPRS_INT_MAGIC || p->i_gap != ((intptr_t) p & 0xffff))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  return p->i_val;
}				/* end bprs_get_int */

uint32_t
bprs_hash_int (const Bprs_Int *p, bool *pok)
{
  if (!p || p->i_kind != BprsKind_Int
      || p->i_magic != BPRS_INT_MAGIC
      || p->i_gap != ((intptr_t) p & 0xffff))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  uint32_t h = (uint32_t) p->i_val;
  if (h == 0)
    h = ((p->i_val % 45233) & 0xfffff) + 2;
  assert (h != 0);
  return h;
}				/* end bprs_hash_int */


Bprs_Double *
bprs_make_double (double d)
{
  Bprs_Double *p = GC_MALLOC_ATOMIC (sizeof (Bprs_Double));
  p->d_kind = BprsKind_Double;
  p->d_gap = (intptr_t) p & BPRS_MASK_DOUBLE;
  p->d_magic = BPRS_DOUBLE_MAGIC;
  p->d_val = d;
  return p;
}				/* end bprs_make_double */

double
bprs_get_double (const Bprs_Double *p, bool *pok)
{
  if (!p || p->d_kind != BprsKind_Double
      || p->d_magic != BPRS_DOUBLE_MAGIC
      || p->d_gap != ((intptr_t) p & BPRS_MASK_DOUBLE))
    {
      if (pok)
	*pok = false;
      return __builtin_nan ("");
    };
  if (pok)
    *pok = true;
  return p->d_val;
}				/* end bprs_get_double */

uint32_t
bprs_hash_double (const Bprs_Double *p, bool *pok)
{
  uint32_t h = 0;
  if (!p || p->d_kind != BprsKind_Double
      || p->d_magic != BPRS_DOUBLE_MAGIC
      || p->d_gap != ((intptr_t) p & BPRS_MASK_DOUBLE))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  if (isnan (p->d_val))
    return 17;
  else if (isinf (p->d_val))
    return 45413;
  h = (uint32_t) round (log (fabs (p->d_val) + 0.0001) * 65.637);
  if (h == 0)
    h = 17;
  return h;
}				/* end bprs_hash_double */
