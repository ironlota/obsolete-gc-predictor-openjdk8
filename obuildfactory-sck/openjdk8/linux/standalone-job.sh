#!/bin/sh
#

## @cesar: I added this flag here dont update always!
JUST_BUILD="--just-build"

export OBF_PROJECT_NAME=openjdk8
TAG_FILTER=jdk8

#
# Safe Environment
#
export LC_ALL=C
export LANG=C

#
# Prepare Drop DIR
#
if [ -z "$OBF_DROP_DIR" ]; then
  export OBF_DROP_DIR=`pwd`/OBF_DROP_DIR
fi

#
# Provide Main Variables to Scripts
#
if [ -z "$OBF_BUILD_PATH" ]; then
  export OBF_BUILD_PATH=`pwd`/obuildfactory-sck/$OBF_PROJECT_NAME/linux
fi

if [ -z "$OBF_SOURCES_PATH" ]; then
  export OBF_SOURCES_PATH=`pwd`/sources/$OBF_PROJECT_NAME
  mkdir -p `pwd`/sources
fi

if [ -z "$OBF_WORKSPACE_PATH" ]; then
  export OBF_WORKSPACE_PATH=`pwd`
fi

if [ ! -d $OBF_SOURCES_PATH ]; then
  hg clone http://hg.openjdk.java.net/jdk8u/jdk8u $OBF_SOURCES_PATH
fi

pushd $OBF_SOURCES_PATH >>/dev/null

if [ ! "$1" == "$JUST_BUILD" ]; then

  #
  # Updating sources for Mercurial repo
  #
  sh ./get_source.sh

  if [ -n "$XUSE_UPDATE" ]; then
    XUSE_TAG=`hg tags | grep "jdk8u$XUSE_UPDATE" | head -1 | cut -d ' ' -f 1`
  fi

  #
  # Update sources to provided tag XUSE_TAG (if defined)
  #
  if [ ! -z "$XUSE_TAG" ]; then
    echo "using tag $XUSE_TAG"
    sh ./make/scripts/hgforest.sh update --clean $XUSE_TAG
  fi

  popd >>/dev/null
fi

#
# Mercurial repositories updated, call Jenkins job now
#
bash $OBF_BUILD_PATH/jenkins-job.sh
