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
