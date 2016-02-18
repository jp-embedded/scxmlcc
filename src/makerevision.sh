#!/bin/sh
. ./autorevision.mk

# Generate version.h - the scxmlcc version
# This file is stored in version control
# Could be updated automatically if on a release branch Otherwise, leave it
#echo '#define VERSION "0.6.2"' > autorevision.h~
#diff -q version.h~ version.h || cp version.h~ version.h

# Generate version_auto.h
# This file is NOT stored in vorsion control because it is only valid for current build
# - if not on version control (code downloaded from zip): no postfix
# - if on master branch: use -$VCS_TICK, if not 0
# - otherwise: Use '-$FULL_HASH'
# - add M if locally modified
mod=""
if [ "$VCS_WC_MODIFIED" == "1" ]; then
  mod="M"
fi
if [ "$VCS_BRANCH" == "master" ]; then
   if [ "$VCS_TICK" == "0" -a "$mod" == "" ]; then
      truncate -s0 version_auto.h~
   else
      echo '#define VERSION_POSTFIX "-'${VCS_TICK}''${mod}'"' > version_auto.h~
   fi
else
   echo '#define VERSION_POSTFIX "-'${VCS_FULL_HASH}''${mod}'"' > version_auto.h~
fi
diff -q version_auto.h~ version_auto.h || cp version_auto.h~ version_auto.h
