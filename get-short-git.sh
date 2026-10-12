#!/bin/sh
git log -1 $1 | awk '/commit/{printf "%.10s\n",$2}' 
