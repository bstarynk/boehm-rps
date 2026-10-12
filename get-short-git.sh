#!/bin/sh
# SPDX-License-Identifier: LGPL-3.0-or-later
#
#      © Copyright (C) 2026 Basile Starynkevitch, France
# see 
gitstr=$(git log -1 $1 | awk '/commit/{printf "%.10s\n",$2}')
if [ -n "$(git status --short --porcelain $1)" ]; then
    gitstr="$gitstr+"
fi
echo $gitstr
