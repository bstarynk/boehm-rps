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

static int _bprs_argc;
static char **_bprs_argv;
static void *_bprs_proghdl;
static char _bprs_cwdbuf[256];

static const struct option _bprs_options[] = {
  {.name = (const char *) "help",
   .has_arg = no_argument,
   .flag = NULL,
   .val = 'H'},
  {.name = (const char *) "version",
   .has_arg = no_argument,
   .flag = NULL,
   .val = 'V'},
  {.name = (const char *) "load",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'L'},
  {.name = (const char *) "plugin",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'P'},
  {.name = (const char *) 0,.has_arg = 0,.flag = NULL,.val = 0}
};

static void
bprs_show_version (void)
{
  typedef const char *gitfun_sigt (void);
  printf ("version of %s\n", _bprs_argv[0]);
  printf ("\t global git %s\n", bprs_shortgit);
  printf ("\t source directory %s\n", bprs_sourcedir);
  for (const char **pfil = bprs_human_files; *pfil; pfil++)
    {
      char nambuf[80];
      memset (nambuf, 0, sizeof (nambuf));
      snprintf (nambuf, sizeof (nambuf), "bprs_shortgit_%.30s", *pfil);
      char *dot = strchr (nambuf, '.');
      if (dot)
	*dot = (char) 0;
      void *ad = dlsym (_bprs_proghdl, nambuf);
      if (ad)
	printf ("\t %-16s : %s\n", *pfil, (*(gitfun_sigt *) ad) ());
      else
	printf ("\t %-16s : ??\n", *pfil);
    };
  fflush (stdout);
}				/* end bprs_show_version */

static void
bprs_show_usage (void)
{
  printf ("usage of %s:\n", _bprs_argv[0]);
  printf ("\t -V | --version        # show version information\n");
  printf ("\t -H | --help           # show this help\n");
  printf ("\t -L | --load $FILE     # load the given textual $FILE\n");
  printf ("\t -P | --plugin $PLUGIN # dlopen given $PLUGIN\n");
}				/* end of bprs_show_usage */

static void
_bprs_process_progargs (int argc, char **argv)
{
  int argcod = -1;
  int optix = 0;
  for (;;)
    {
      argcod = getopt_long (argc, argv, "VHL:P:", _bprs_options, &optix);
      switch (argcod)
	{
	case 'V':
	  bprs_show_version ();
	  exit (EXIT_SUCCESS);
	  break;
	case 'H':
	  bprs_show_usage ();
	  exit (EXIT_SUCCESS);
	case 'P':
	  {
	    void *dlh = dlopen (optarg, RTLD_NOW | RTLD_GLOBAL);
	    if (!dlh)
	      BPRS_FATAL ("failed to dlopen %s: %s", optarg, dlerror ());
	    continue;
	  }
	};
      if (optix > argc)
	return;
    };
}				/* end bprs_process_progargs */

int
main (int argc, char **argv)
{
  GC_INIT ();
  _bprs_argc = argc;
  _bprs_argv = argv;
  openlog ("boehm-rps", LOG_CONS | LOG_PID, LOG_USER);
  _bprs_proghdl = dlopen (NULL, RTLD_NOW);
  if (!_bprs_proghdl)
    BPRS_FATAL ("failed to dlopen self program (%s)\n", dlerror ());
  getcwd (_bprs_cwdbuf, sizeof (_bprs_cwdbuf) - 1);
  syslog (LOG_INFO, "starting %s git %s in %s (srcdir %s)",
	  _bprs_argv[0], bprs_shortgit, _bprs_cwdbuf, bprs_sourcedir);
  _bprs_process_progargs (argc, argv);
}				/* end main */

int
brps_argc (void)
{
  return _bprs_argc;
}				/* end of brps_argc */

const char *
brps_progargv (int rk)
{
  if (rk < 0)
    rk += _bprs_argc;
  if (rk >= 0 && rk < _bprs_argc)
    return _bprs_argv[rk];
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
    default:
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
