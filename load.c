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


#define BPRS_LOAD_MAGIC 0x382237B	//58860411
struct bprs_loaded_file_st	/* ie Bprs_Loaded_File */
{
  unsigned ldf_magic;		/* should be BPRS_LOAD_MAGIC */
  unsigned ldf_lineno;		/* line number */
  unsigned ldf_linoff;		/* current offset in line */
  int ldf_linelen;
  char *ldf_linbuf;		/* malloced line buffer, thru getline */
  size_t ldf_bufsiz;		/* current size of ldf_linbuf */
  const char *ldf_path;		/* if starts with ! or | use popen */
  FILE *ldf_file;
  void *ldf_data;
  void *ldf_xtra;
};

Bprs_Loaded_File *
bprs_open_load_file (const char *path)
{
  Bprs_Loaded_File *res = NULL;
  if (!path || !path[0])
    BPRS_FATAL ("invalid path to bprs_open_load_file");
  res = GC_MALLOC (sizeof (*res));
  memset (res, 0, sizeof (*res));
  res->ldf_magic = BPRS_LOAD_MAGIC;
  if (path[0] == '!' || path[0] == '|')
    res->ldf_file = popen (path + 1, "r");
  else
    res->ldf_file = fopen (path, "r");
  if (!res->ldf_file)
    BPRS_FATAL ("failed to open load file %s", path);
  {
    char*repath = realpath (path, NULL);
    assert (repath != NULL);
    res->ldf_path = GC_STRDUP (repath);
    free (repath);
  }
  assert (res->ldf_path);
  ssize_t linlen =
    getline (&res->ldf_linbuf, &res->ldf_bufsiz, res->ldf_file);
  if (linlen > INT_MAX / 4)
    BPRS_FATAL ("too long %ld first line in %s", (long) linlen, path);
  res->ldf_linelen = (int) linlen;
  /// find first a line having  "Boehm-Rps-Store"
  while ((linlen =
	  getline (&res->ldf_linbuf, &res->ldf_bufsiz, res->ldf_file)) > 0)
    {
      res->ldf_lineno++;
      if (strstr (res->ldf_linbuf, BPRS_MAGIC_STRING))
	break;
      if (linlen > INT_MAX / 4)
	BPRS_FATAL ("too long %ld  line#%d in %s",
		    (long) linlen, res->ldf_lineno, res->ldf_path);
      res->ldf_linelen = (int) linlen;
    };
  if (feof (res->ldf_file))
    BPRS_FATAL ("no line with magic %s in load file %s",
		BPRS_MAGIC_STRING, res->ldf_path);
  return res;
}				/* end bprs_open_load_file */

void
bprs_process_loaded_file (Bprs_Loaded_File *ld)
{
  if (!ld || ld->ldf_magic != BPRS_LOAD_MAGIC)
    BPRS_FATAL ("invalid argument @%p to bprs_process_loaded_file",
		(void *) ld);
  if (!ld->ldf_linbuf || !ld->ldf_file || !ld->ldf_path)
    BPRS_FATAL ("corruption in bprs_process_loaded_file ld@%p", ld);
  if (bprs_verbose)
    printf ("%s process loaded file %s:%d\n",
	    brps_progargv (0), ld->ldf_path, ld->ldf_lineno);
#warning incomplete bprs_process_loaded_file
  BPRS_WARNING ("incomplete bprs_process_loaded_file %s", ld->ldf_path);
}				/* end bprs_process_loaded_file */

#warning we could need bprs_load_object_content
/* we might need a struct packing load data */

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
      const int leftlen = 32;
      const int rightlen = 40;
      char bufleft[leftlen + 4];
      char bufright[rightlen + 4];
      Bprs_Object *pobj = NULL;
      int pos = 0;
      const char *pc = 0;
      memset (linebuf, 0, linsiz);
      readsiz = getline (&linebuf, &linsiz, fil);
      memset (bufleft, 0, sizeof (bufleft));
      memset (bufright, 0, sizeof (bufright));
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
	  pobj = bprs_object_find (hi, lo);
	  if (!pobj)
	    {
	      bool ok = false;
	      pobj = bprs_object_load (fil, hi, lo);
	      if (bprs_value_kind (pobj) != BprsKind_Object || !ok)
		BPRS_FATAL ("in %s:%d  for object hi=%d lo=%ld "
			    "bad hash %d",
			    name, linecnt, hi, lo,
			    bprs_hash_object (pobj, NULL));
	    }
	  else
	    {
	      BPRS_WARNING ("in %s:%d already existing object hi=%d lo=%ld",
			    name, linecnt, hi, lo);
	    }
	}			/* end @obj */
      else if (sscanf (pc, " @Ob_%32[0-9A-Z]_%40[0-9A-Z]%n",
		       bufleft, bufright, &pos) >= 2 && pos > 0)
	{
	  bool okhi = false, oklo = false, ok = false;
	  const char *endleft = NULL;
	  const char *endright = NULL;
	  hi = (uint32_t) bprs_b36_to_uint64 (bufleft, &endleft, &okhi);
	  if (!endleft)
	    okhi = false;
	  lo = bprs_b36_to_uint64 (bufright, &endright, &oklo);
	  if (!endright)
	    oklo = false;
	  pc += pos;
	  uint32_t hx = bprs_hash_for_object_hi_lo (hi, lo);
	  if (!hx)
	    BPRS_FATAL ("%s:%d invalid hi=%lu lo=%lu", name, linecnt,
			(uint64_t) hi, (uint64_t) lo);
	  pobj = bprs_object_load (fil, hi, lo);
	  if (bprs_value_kind (pobj) != BprsKind_Object || !ok)
	    BPRS_FATAL ("in %s:%d  for object hi=%d lo=%ld "
			"bad hash %d",
			name, linecnt, hi, lo, bprs_hash_object (pobj, NULL));

	  BPRS_FATAL ("%s:%d missing code bufleft=%s bufright=%s (%s:%d)",
		      name, linecnt, bufleft, bufright, __FILE__, __LINE__);
	}
#warning bprs_load_file incomplete
      BPRS_FATAL ("incomplete bpr_load_file [%s:%d] at loaded file %s:%d",
		  __FILE__, __LINE__, name, linecnt);
    }
  while (readsiz > 0 && !feof (fil));
}				/* end bprs_load_file */

////////// end of file boehm-rps/load.c
