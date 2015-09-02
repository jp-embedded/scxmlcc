#!/bin/sh
. ./autorevision.mk

# Generate autoversion.h - the scxmlcc version
# This file is stored in version control
# todo: generate version if on release branch. Otherwise, leave it
echo '#define VERSION "0.4.4"' > autorevision.h~
diff -q autorevision.h~ autorevision.h || cp autorevision.h~ autorevision.h

# Generate autoversion_postfix.h
# This file is NOT stored in vorsion control because it is only valid for current build
# - if not on version control (code downloaded from zip): no postfix
# - if on master branch: no postfix
# - otherwise: Use '-SNAPSHOTXX'
# todo: test branch/non-vcs
echo '#define VERSION_POSTFIX "-'${VCS_FULL_HASH}'"' > autorevision_postfix.h~
diff -q autorevision_postfix.h~ autorevision_postfix.h || cp autorevision_postfix.h~ autorevision_postfix.h
