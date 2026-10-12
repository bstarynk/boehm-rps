/****************************************************************
 * file boehmrps/boehmrps.h
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Author(s):
 *      Basile Starynkevitch, France   <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****************************************************************/
#ifndef BOEHMRPS_INCLUDED
#define BOEHMRPS_INCLUDED 1

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <gc.h>

#ifndef BPRS_SHORTGIT
#error compile commnd should define BPRS_SHORTGIT
#endif

/// convention: our human written functions are named brps_*


//// functions in main.c:::::
extern int brps_argc (void);
extern const char *brps_progargv (int rk);
extern const char *bprs_shortgit_main (void);
//// end of functions in main.c

#endif /*BOEHMRPS_INCLUDED */
