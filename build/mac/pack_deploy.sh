#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# Override by envrionment variable for another target
export SSH_DEPLOY_TARGET=${SSH_DEPLOY_TARGET:-"sol:/data/alex/Public/Releases"}

(
  cd ${APROJECTS}/deploy

  rm -rfv LittleXpconnect.zip

  zip -r -y -9 LittleXpconnect.zip "Little Xpconnect"

  export FILENAME_LXP=$(head -n1 "${APROJECTS}/deploy/Little Xpconnect/version.txt")

  scp LittleXpconnect.zip ${SSH_DEPLOY_TARGET}/LittleXpconnect-macOS-${FILENAME_LXP}_x86_arm64.zip
)
