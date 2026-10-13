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

////////// end of file boehm-rps/load.c
