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

extern void bprs_last_main (void);

const char *
bprs_shortgit_main (void)
{
  assert ((char *) bprs_last_main > (char *) bprs_shortgit_main);
  return _bprs_main_shortgit;
}				/* end bprs_shortgit_main */

bool bprs_verbose;
static int _bprs_argc;
static char **_bprs_argv;
static void *_bprs_proghdl;
static char _bprs_cwdbuf[256];
static long _bprs_random;

static const struct option _bprs_options[] = {
  {.name = (const char *) "help",
   .has_arg = no_argument,
   .flag = NULL,
   .val = 'H'},
  {.name = (const char *) "version",
   .has_arg = no_argument,
   .flag = NULL,
   .val = 'V'},
  {.name = (const char *) "verbose",
   .has_arg = no_argument,
   .flag = NULL,
   .val = 'v'},
  {.name = (const char *) "load",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'L'},
  {.name = (const char *) "plugin",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'P'},
  {.name = (const char *) "random-seed",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'R'},
  {.name = (const char *) "show-oids",
   .has_arg = required_argument,
   .flag = NULL,
   .val = 'O'},
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
  puts ("\t -v | --verbose           # verbose run");
  puts ("\t -V | --version           # show version information");
  puts ("\t -H | --help              # show this help");
  puts ("\t -L | --load $FILE        # load the given textual $FILE");
  puts ("\t -P | --plugin $PLUGIN    # dlopen given $PLUGIN");
  puts ("\t -R | --random-seed $HEX  # $HEX is an hexadecimal string");
  puts ("\t                          # for initstate(3) of random(3)");
  puts ("\t -O | --show-oids $NUM    # show $NUM fresh oids");
}				/* end of bprs_show_usage */

static void
_bprs_process_progargs (int argc, char **argv)
{
  int argcod = -1;
  int optix = 0;
  int loopcnt = 0;
  for (;;)
    {
      loopcnt++;
      if (loopcnt > argc + 2)
	BPRS_FATAL ("too many %d loops for %d program arguments (git %s)",
		    loopcnt, argc, bprs_shortgit);
      argcod = getopt_long (argc, argv, "vVHL:P:R:O:",	/// optix may change
			    _bprs_options, &optix);
      switch (argcod)
	{
	case 'v':
	  bprs_verbose = true;
	  printf ("%s (git %s) runs verbosely (pid %d) on %s [%s:%d]\n",
		  argv[0], bprs_shortgit, (int) getpid (),
		  bprs_host (), __FILE__, __LINE__);
	  break;
	case 'V':
	  bprs_show_version ();
	  exit (EXIT_SUCCESS);
	  break;
	case 'H':
	  bprs_show_usage ();
	  exit (EXIT_SUCCESS);
	case 'O':
	  {
	    char bu[40];
	    int nb = atoi (optarg);
	    if (nb <= 0)
	      nb = 1;
	    else if (nb > 32)
	      nb = 32;
	    memset (bu, 0, sizeof (bu));
	    uint64_t k = 36L * 36 * 36L * 36 * 36L * 36 * 36L * 36 * 35;
	    if (bprs_uint64_to_b36 (k, bu, sizeof (bu) - 1))
	      printf ("%ld : %s [%s:%d]\n", k, bu, __FILE__, __LINE__);
	    uint64_t kb = 36L * 36 * 36 * 36;
	    if (bprs_uint64_to_b36 (kb, bu, sizeof (bu) - 1))
	      printf ("%ld : %s [%s:%d] /*36^4*/\n", kb, bu,
		      __FILE__, __LINE__);
	    assert (strtol (bu, NULL, 36) == (long) kb);
	    fflush (NULL);
	    for (int ix = 0; ix < nb; ix++)
	      {
		char buf[48];
		uint32_t hi = 0;
		uint64_t lo = 0;
		memset (buf, 0, sizeof (buf));
		Bprs_Object *pob =
		  bprs_object_make ( /*compsize: */ 5 + (ix & 0xf),
				    /*attrsize */ (7 + (ix & 7)));
		if (bprs_extract_object_id (pob, &hi, &lo))
		  {
		    bprs_object_to_str (pob, buf, sizeof (buf) - 1);
		    printf ("object %s hi=%d lo=%ld\n", buf, hi, lo);
		    fflush (NULL);
		  }
		else
		  BPRS_WARNING ("failed to extract id from pob@%p", pob);
	      }
	  }
	  break;
	case 'P':
	  {
	    void *dlh = dlopen (optarg, RTLD_NOW | RTLD_GLOBAL);
	    if (!dlh)
	      BPRS_FATAL ("failed to dlopen %s: %s", optarg, dlerror ());
	    continue;
	  }
	case 'L':
	  {
	    Bprs_Loaded_File *ldfil = bprs_open_load_file (optarg);
	    if (!ldfil)
	      BPRS_FATAL ("failed to open load file %s", optarg);
	    bprs_process_loaded_file (ldfil);
	    continue;
	  }
	  break;
	case 'R':
	  {
	    const int randsiz = 256;
	    char randbuf[randsiz + 16];
	    int i = 0;
	    memset (randbuf, 0, sizeof (randbuf));
	    if (optarg[0] == '_')
	      {
		srandom (getpid ());
	      }
	    else
	      {
		for (const char *pc = optarg;
		     i < randsiz
		     && pc && pc[0] && isxdigit (pc[0])
		     && pc[1] && isxdigit (pc[1]); (pc += 2), (i++))
		  {
		    char hxbuf[4];
		    char *endbuf = NULL;
		    hxbuf[0] = pc[0];
		    hxbuf[1] = pc[1];
		    hxbuf[2] = hxbuf[3] = 0;
		    unsigned long hb = strtol (hxbuf, &endbuf, 16);
		    assert (endbuf == hxbuf + 2);
		    randbuf[i] = (char) (hb & 0xff);
		  };
		if (i < 8)
		  BPRS_FATAL ("too short or bad random initializer %s\n"
			      " - eight bytes in hexa required but got %d",
			      optarg, i);
		_bprs_random = strtol (optarg, NULL, 16);
		if (!initstate (_bprs_random, randbuf, (size_t) i))
		  BPRS_FATAL ("initstate %s failed (random %ld)", optarg,
			      _bprs_random);
	      };
	    _bprs_random = random ();
	    if (bprs_verbose)
	      printf ("%s initialized random with %s first random %ld\n",
		      _bprs_argv[0], optarg, _bprs_random);
	    continue;
	  }
	};
      if (optix > argc)
	return;
    };
}				/* end bprs_process_progargs */

static char *_bprs_execpath;
static char _bprs_hostname[80];

const char *
bprs_executable (void)
{
  char buf[384];
  if (_bprs_execpath && _bprs_execpath[0])
    return _bprs_execpath;
  memset (buf, 0, sizeof (buf));
  ssize_t rsiz = readlink ("/proc/self/exe", buf, sizeof (buf) - 1);
  if (rsiz > 0 && buf[0])
    {
      _bprs_execpath = GC_MALLOC_ATOMIC (1 + ((rsiz + 15) | 0xf));
      strncpy (_bprs_execpath, buf, rsiz);
      return _bprs_execpath;
    }
  else
    BPRS_FATAL ("bprs_executable failed to readlink /proc/self/exe %s",
		strerror (errno));
}				/* end bprs_executable */

const char *
bprs_host (void)
{
  return _bprs_hostname;
}				/* end bprs_host */

int
main (int argc, char **argv)
{
  GC_INIT ();
  memset (_bprs_hostname, 0, sizeof (_bprs_hostname));
  _bprs_argc = argc;
  _bprs_argv = argv;
  if (gethostname (_bprs_hostname, sizeof (_bprs_hostname) - 1))
    BPRS_FATAL ("%s: gethostname failed git %s (%s)",
		argv[0], _bprs_main_shortgit, strerror (errno));
  openlog (argv[0], LOG_CONS | LOG_PID, LOG_USER);
  _bprs_proghdl = dlopen (NULL, RTLD_NOW);
  if (!_bprs_proghdl)
    BPRS_FATAL ("failed to dlopen self program (%s)\n", dlerror ());
  char *cwd = getcwd (_bprs_cwdbuf, sizeof (_bprs_cwdbuf) - 1);
  if (!cwd)
    BPRS_FATAL ("getcwd failed in %s git %s (%s:%d)",
		_bprs_argv[0], bprs_shortgit, __FILE__, __LINE__);
  const char *exe = bprs_executable ();
  syslog (LOG_INFO, "starting %s git %s in %s exe %s (srcdir %s)",
	  _bprs_argv[0], bprs_shortgit, _bprs_cwdbuf, exe, bprs_sourcedir);
  bprs_initialize_objects ();
  _bprs_process_progargs (argc, argv);
  if (bprs_verbose)
    printf ("%s: end of process %d git %s host %s (%s:%d)\n",
	    _bprs_argv[0],
	    (int) getpid (), bprs_shortgit, bprs_host (),
	    __FILE__, __LINE__ - 3);
  return 0;
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
    case BprsKind_Set:
      if (bprs_hash_set ((Bprs_Set *) p, &ok) != 0 && ok)
	return BprsKind_Set;
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
    case BprsKind_Object:
      h = bprs_hash_object ((Bprs_Object *) p, &ok);
      break;
    case BprsKind_Set:
      h = bprs_hash_set ((Bprs_Set *) p, &ok);
      break;
    default:
      return 0;
    }
  if (!ok)
    return 0;
  return h;
}				/* end of bprs_value_hash  */

#if 0 && redefinition_malloc
extern void *malloc (size_t siz);
extern void free (void *);
extern void *calloc (size_t nbelem, size_t siz);
extern void *realloc (void *p, size_t siz);
extern void *reallocarray (void *p, size_t nbelem, size_t siz);

void *
malloc (size_t siz)
{
  if (siz == 0)
    return NULL;
  if (siz % 16 != 0)
    siz = (siz | 0xf) + 1;
  void *p = GC_MALLOC (siz);
  assert (p != NULL);
  memset (p, 0, siz);
  return p;
}				/* end malloc */

void
free (void *p)
{
  if (p)
    GC_FREE (p);
}				/* end free */

void *
calloc (size_t nbelem, size_t elemsiz)
{
  if (nbelem == 0)
    return NULL;
  assert (elemsiz > 0);
  if (nbelem > (1 << 30) || elemsiz > (1 << 20))
    BPRS_FATAL ("calloc nbelem=%ld elemsiz=%ld too big",
		(long) nbelem, (long) elemsiz);
  if ((((unsigned long) nbelem * (unsigned long) elemsiz) >> 10)
      < (32 << 20) /* 32 Gigabytes */ )
    BPRS_FATAL ("calloc nbelem=%ld elemsiz=%ld too much",
		(long) nbelem, (long) elemsiz);
  void *p = GC_MALLOC (nbelem * elemsiz);
  if (p)
    memset (p, 0, nbelem * elemsiz);
  return p;
}				/* end calloc */

void *
realloc (void *p, size_t siz)
{
  if ((siz >> 10) > (32 << 20))
    BPRS_FATAL ("realloc too big %ld kbytes", (siz >> 10));
  void *res = GC_REALLOC (p, siz);
  return res;
}				/* end realloc */


void *
reallocarray (void *p, size_t nbcomp, size_t elemsize)
{
  if ((((unsigned long) nbcomp * (unsigned long) elemsize) >> 10)
      < (32 << 20) /* 32 Gigabytes */ )
    BPRS_FATAL ("reallocarray p=%p nbcomp=%ld elemsize=%ld too much",
		p, (long) nbcomp, (long) elemsize);
  void *res = GC_REALLOC (p, nbcomp * elemsize);
  return res;
}				/* end reallocarray */
#endif /*0 && redefinition_malloc */




void
bprs_last_main (void)
{
  asm volatile ("nop; nop; nop; nop");
}				/* end bprs_last_main */

/****************
 **                           for GNU Emacs...
 ** Local Variables: ;;
 ** compile-command: "make -j2" ;;
 ** End: ;;
 ****************/

///// end of file boehm-rps/main.c (on github.com/bstarynk)
