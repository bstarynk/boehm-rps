#!/bin/bash
# file boehm-rps/do-load-boehmrps.sh
# SPDX-License-Identifier: GPL-3.0-or-later
# useless stuff

# Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
#      <basile@starynkevitch.net>
#
#      © Copyright (C) 2026 Basile Starynkevitch
#
if [ ! -x ./rpsboehm ] ; then
    printf "%s building rpsboehm from code on github.com/bstarynk/boehm-rps.git in %s\n" \
	   "$0" "$(/bin/pwd)"
    make
fi
./rpsboehm -v -L $0
exit $?
# our magic string is below
Boehm-Rps-Store
