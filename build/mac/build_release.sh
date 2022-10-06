#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# =============================================================================
# Set the required environment variable APROJECTS to the base directory for
# atools, littlenavmap, littlenavconnect and littlexpconnect.

# =============================================================================
# Configuration can be overloaded on the command line by defining the
# variables below before calling this script.
#
# See the *.pro project files for more information.
#
# Example:
# export MARBLE_LIB_PATH=/home/alex/Programme/Marble-debug/lib
# export MARBLE_INC_PATH=/home/alex/Programme/Marble-debug/include

export CONF_TYPE=${CONF_TYPE:-"release"}
export ATOOLS_INC_PATH=${ATOOLS_INC_PATH:-"${APROJECTS}/atools/src"}
export ATOOLS_LIB_PATH=${ATOOLS_LIB_PATH:-"${APROJECTS}/build-atools-${CONF_TYPE}"}
export XPSDK_BASE=${XPSDK_BASE:-"${APROJECTS}/X-Plane SDK"}

export ATOOLS_NO_FS=true
export ATOOLS_NO_GRIB=true
export ATOOLS_NO_GUI=true
export ATOOLS_NO_ROUTING=true
export ATOOLS_NO_SQL=true
export ATOOLS_NO_TRACK=true
export ATOOLS_NO_USERDATA=true
export ATOOLS_NO_WEATHER=true
export ATOOLS_NO_WEB=true
export ATOOLS_NO_WMM=true

# Defines the used Qt for all builds
export QMAKE_SHARED=${QMAKE_SHARED:-"qmake"}

# Do not change the DEPLOY_BASE since some scripts depend on it
export DEPLOY_BASE="${APROJECTS}/deploy"

# ===========================================================================
# ========================== atools
rm -rf ${APROJECTS}/build-atools-${CONF_TYPE}
mkdir -p ${APROJECTS}/build-atools-${CONF_TYPE}
cd ${APROJECTS}/build-atools-${CONF_TYPE}

${QMAKE_SHARED} ${APROJECTS}/atools/atools.pro -spec macx-clang CONFIG+=${CONF_TYPE} 'QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64'
make -j4

# ===========================================================================
# ========================== littlexpconnect
rm -rf ${APROJECTS}/build-littlexpconnect-${CONF_TYPE}
mkdir -p ${APROJECTS}/build-littlexpconnect-${CONF_TYPE}
cd ${APROJECTS}/build-littlexpconnect-${CONF_TYPE}

${QMAKE_SHARED} ${APROJECTS}/littlexpconnect/littlexpconnect.pro -spec macx-clang CONFIG+=${CONF_TYPE} 'QMAKE_APPLE_DEVICE_ARCHS=x86_64 arm64'
make -j4
make deploy -i -l



