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
  memset (linebuf, 0, linsiz);
  readsiz = getline (&linebuf, &linsiz, fil);
}				/* end bprs_load_file */

////////// end of file boehm-rps/load.c
