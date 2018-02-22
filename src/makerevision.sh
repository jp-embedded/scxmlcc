#!/bin/sh
. ./autorevision.mk

# Generate version_auto.h
# This file is NOT stored in vorsion control because it is only valid for current build
# - if not on version control (code downloaded from zip): no postfix
# - if on master branch: use -$VCS_TICK, if not 0
# - otherwise: Use '-$FULL_HASH'
# - add M if locally modified
mod=""
if [ "$VCS_WC_MODIFIED" = "1" ]; then
  mod="M"
fi
if [ "$VCS_BRANCH" = "master" ]; then
   if [ "$VCS_TICK" = "0" -a "$mod" = "" ]; then
      truncate -s0 version_auto.h~
   else
      echo '#define VERSION_POSTFIX "'${mod}'-'${VCS_TICK}'"' > version_auto.h~
   fi
else
   echo '#define VERSION_POSTFIX "'${mod}'-'${VCS_FULL_HASH}'"' > version_auto.h~
fi
diff -q version_auto.h~ version_auto.h || cp version_auto.h~ version_auto.h
