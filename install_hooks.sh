#!/bin/sh
set -e
top_dir=$(git rev-parse --show-toplevel)
rm -f ${top_dir}/.git/hooks/post-commit
ln -sr ${top_dir}/hooks/post-commit ${top_dir}/.git/hooks/

