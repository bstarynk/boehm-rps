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

static const char _bprs_load_shortgit[] = BPRS_SHORTGIT;

const char *
bprs_shortgit_load (void)
{
  return _bprs_load_shortgit;
}				/* end bprs_shortgit_load */

void
bprs_load_file (const char *name, FILE *fil)
{
  assert (fil != NULL && __freadable (fil));
  char *linebuf = NULL;
  int linecnt = 0;
  size_t linsiz = 0;
  ssize_t readsiz = 0;
  /// find first a line having  "Boehm-Rps-Store"
  while ((readsiz = getline (&linebuf, &linsiz, fil)) > 0)
    {
      linecnt++;
      if (strstr (linebuf, BPRS_MAGIC_STRING))
	break;
    };
  if (feof (fil))
    BPRS_FATAL ("loaded file %s:%d without magic string %s\n",
		name, linecnt, BPRS_MAGIC_STRING);
  do
    {
      uint32_t hi = 0;
      uint64_t lo = 0;
      Bprs_Object *pobj = NULL;
      int pos = 0;
      const char *pc = 0;
      memset (linebuf, 0, linsiz);
      readsiz = getline (&linebuf, &linsiz, fil);
      pc = linebuf;
      if (sscanf (pc, " @obj hi %d lo %ld %n",
		  &hi, &lo, &pos) >= 2 && pos > 0)
	{
	  uint32_t hx = bprs_hash_for_object_hi_lo (hi, lo);
	  uint32_t hread = 0;
	  int posbis = 0;
	  if (hx == 0)
	    BPRS_FATAL ("in %s:%d invalid hi=%d lo=%ld [%s:%d]",
			name, linecnt, hi, lo, __FILE__, __LINE__);
	  pc += pos;
	  if (sscanf (pc, " hash %d %n", &hread, &posbis) > 0 && posbis > 0)
	    {
	      if (hread != hx)
		BPRS_FATAL ("in %s:%d  for object hi=%d lo=%ld "
			    "bad hash %d (expecting %d)",
			    name, linecnt, hi, lo, hread, hx);
	      pc += posbis;
	    }
	}
      pobj = bprs_object_find (hi, lo);
      if (!pobj)
	{
	  bool ok = false;
	  pobj = bprs_object_load (fil, hi, lo);
	  if (bprs_value_kind (pobj) != BprsKind_Object || !ok)
	    BPRS_FATAL ("in %s:%d  for object hi=%d lo=%ld "
			"bad hash %d",
			name, linecnt, hi, lo, bprs_hash_object (pobj, NULL));
	}
#warning bprs_load_file incomplete
      BPRS_FATAL ("incomplete bpr_load_file [%s:%d] at loaded file %s:%d",
		  __FILE__, __LINE__, name, linecnt);
    }
  while (readsiz > 0 && !feof (fil));
}				/* end bprs_load_file */

////////// end of file boehm-rps/load.c
