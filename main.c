/****************************************************************
 * file boehm-rps/main.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description: This file is part of the Reflective Persistent System
 *      (Boehm variant) It is its main function
 *
 * Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
 *      <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****/
#include "boehmrps.h"

static const char _bprs_main_shortgit[] = BPRS_SHORTGIT;

static int _brps_argc;
static char **_brps_argv;
static void *_bprs_proghdl;
int
main (int argc, char **argv)
{
  GC_INIT ();
  _brps_argc = argc;
  _brps_argv = argv;
  openlog ("boehm-rps", LOG_CONS | LOG_PID, LOG_USER);
  _bprs_proghdl = dlopen (NULL, RTLD_NOW);
}				/* end main */

int
brps_argc (void)
{
  return _brps_argc;
}				/* end of brps_argc */

const char *
brps_progargv (int rk)
{
  if (rk < 0)
    rk += _brps_argc;
  if (rk >= 0 && rk < _brps_argc)
    return _brps_argv[rk];
  return NULL;
}				/* end of brps_argc */

const char *
bprs_shortgit_main (void)
{
  return _bprs_main_shortgit;
}				/* end bprs_shortgit_main */

void *
bprs_program_dlhandle (void)
{
  return _bprs_proghdl;
}				/* end bprs_program_dlhandle */

void *
bprs_dlsym (const char *name)
{
  if (!name || name[0] == '_' || !isalnum (name[0]))
    {
      BPRS_WARNING ("invalid symbol %s to dlsym", name);
      return NULL;
    }
  return dlsym (_bprs_proghdl, name);
}				/* end bprs_dlsym */

Bprs_Kind
bprs_value_kind (const void *p)
{
  if (!p || ((uintptr_t) p & 0xf))
    return BprsKind__None;
  bool ok = false;
  switch (*(Bprs_Kind *) p)
    {
    case BprsKind_Int:
      if (bprs_hash_int ((Bprs_Int *) p, &ok) != 0 && ok)
	return BprsKind_Int;
      break;
    case BprsKind_Double:
      if (bprs_hash_double ((Bprs_Double *) p, &ok) != 0 && ok)
	return BprsKind_Double;
      break;
    case BprsKind_String:
      if (bprs_hash_string ((Bprs_String *) p, &ok) != 0 && ok)
	return BprsKind_String;
      break;
    case BprsKind_Tuple:
      if (bprs_hash_tuple ((Bprs_Tuple *) p, &ok) != 0 && ok)
	return BprsKind_Tuple;
      break;
    }
  return BprsKind__None;
}				/* end bprs_value_kind */

uint32_t
bprs_value_hash (const void *p)
{
  uint32_t h = 0;
  bool ok = false;
  if (!p || ((uintptr_t) p & 0xf))
    return 0;
  switch (*(Bprs_Kind *) p)
    {
    case BprsKind_Int:
      h = bprs_hash_int ((Bprs_Int *) p, &ok);
      break;
    case BprsKind_Double:
      h = bprs_hash_double ((Bprs_Double *) p, &ok);
      break;
    case BprsKind_String:
      h = bprs_hash_string ((Bprs_String *) p, &ok);
      break;
    case BprsKind_Tuple:
      h = bprs_hash_tuple ((Bprs_Tuple *) p, &ok);
      break;
    default:
      return 0;
    }
  if (!ok)
    return 0;
  return h;
}				/* end of bprs_value_hash  */
