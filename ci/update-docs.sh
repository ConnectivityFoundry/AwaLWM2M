#!/bin/bash -x
# Script to build docs and update FlowM2M/AwaLWM2M-docs repository
#
#  Define DOCKER_RUN_EXTRA_OPTIONS if you wish to add/override container run options.
#
# Assumes FlowM2M/AwaLWM2M-docs is checked out in ../docs
# SSH credentials for pushing to FlowM2M/AwaLWM2M-docs are to be provided externally (e.g. from Jenkins)
#

DOCKER_RUN_EXTRA_OPTIONS=$DOCKER_RUN_EXTRA_OPTIONS

set -o errexit
set -o nounset

SRC_DOCS=$PWD/api/doc/html
DST_DOCS=../docs

# only enable Docker pseudoterminal if a TTY is present:
if [ -t 1 ]; then
  TERMINAL_OPT=-t
else
  TERMINAL_OPT=
fi

# make docs
docker run \
  -v $(pwd):/home/build/AwaLWM2M \
  -w /home/build/AwaLWM2M \
  -i $TERMINAL_OPT \
  $DOCKER_RUN_EXTRA_OPTIONS \
  flowm2m/awalwm2m.ci \
  make docs

GIT_REV=$(git rev-parse HEAD)

(
  cd $DST_DOCS
  git checkout -b gh-pages-update
  git rm -rf --ignore-unmatch *
  cp -a $SRC_DOCS/* .
  git add . --all
  git status
  git commit -m "Update documentation from FlowM2M/AwaLWM2M.git :: $GIT_REV."
  git push origin gh-pages-update:gh-pages
  git checkout origin/gh-pages
  git branch -D gh-pages-update
)
