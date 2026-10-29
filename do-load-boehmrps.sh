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
    make
fi
./rpsboehm -v -L $0
exit $?
# our magic string is below
Boehm-Rps-Store
