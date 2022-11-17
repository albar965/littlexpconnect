#*****************************************************************************
# Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#****************************************************************************

# =============================================================================
# Set these environment variables for configuration - do not change this .pro file
# =============================================================================
#
# ATOOLS_INC_PATH
# Optional. Path to atools include. Default is "../atools/src" if not set.
# Also reads *.qm translation files from "$ATOOLS_INC_PATH/..".
#
# ATOOLS_LIB_PATH
# Optional. Path to atools static library. Default is "../build-atools-$${CONF_TYPE}"
# ("../build-atools-$${CONF_TYPE}/$${CONF_TYPE}" on Windows) if not set.
#
# ATOOLS_GIT_PATH
# Optional. Path to GIT executable. Revision will be set to "UNKNOWN" if not set.
# Uses "git" on macOS and Linux as default if not set.
# Example: "C:\Git\bin\git"
#
# XPSDK_BASE
# Required. Path to X-Plane SDK.
# Example: "C:\X-Plane SDK", "$HOME/Programme/XPSDK" or "/Users/alex/XPSDK"
#
# DEPLOY_BASE
# Optional. Target folder for "make deploy". Default is "../deploy" plus project name ($$TARGET_NAME).
#
# ATOOLS_QUIET
# Optional. Set this to "true" to avoid qmake messages.
#
# =============================================================================
# End of configuration documentation
# =============================================================================

# Define program version here VERSION_NUMBER_TODO
VERSION_NUMBER=1.0.30

QT += core
QT -= gui
versionAtLeast(QT_VERSION, 6.0.0): QT += core5compat

macx {
QT += widgets
}

CONFIG += c++14
CONFIG += dll
CONFIG += build_all c++14
CONFIG -= gui debug_and_release debug_and_release_target

TARGET = littlexpconnect
TEMPLATE = lib

TARGET_NAME=Little Xpconnect

# =======================================================================
# Copy environment variables into qmake variables

ATOOLS_INC_PATH=$$(ATOOLS_INC_PATH)
ATOOLS_LIB_PATH=$$(ATOOLS_LIB_PATH)
GIT_PATH=$$(ATOOLS_GIT_PATH)
XPSDK_BASE=$$(XPSDK_BASE)
DEPLOY_BASE=$$(DEPLOY_BASE)
QUIET=$$(ATOOLS_QUIET)

# =======================================================================
# Fill defaults for unset

CONFIG(debug, debug|release) : CONF_TYPE=debug
CONFIG(release, debug|release) : CONF_TYPE=release

isEmpty(DEPLOY_BASE) : DEPLOY_BASE=$$PWD/../deploy

isEmpty(XPSDK_BASE) : XPSDK_BASE="$$PWD/../X-Plane SDK"
isEmpty(ATOOLS_INC_PATH) : ATOOLS_INC_PATH=$$PWD/../atools/src
isEmpty(ATOOLS_LIB_PATH) : ATOOLS_LIB_PATH=$$PWD/../build-atools-$$CONF_TYPE

# =======================================================================
# Set compiler flags and paths

unix:!macx {
  isEmpty(GIT_PATH) : GIT_PATH=git

LIBS += -lz
  QMAKE_LFLAGS += -static-libstdc++

  # Use relative path to current .so directory to search for shared libraries
  QMAKE_RPATHDIR=.
}

win32 {
  DEFINES += _USE_MATH_DEFINES

  LIBS += -L$${XPSDK_BASE}\Libraries\Win -lXPLM_64 -lXPWidgets_64
}

macx {
  isEmpty(GIT_PATH) : GIT_PATH=git

  QMAKE_RPATHDIR=.

  versionAtLeast(QT_VERSION, 6.0.0) {
    # Compatibility down to OS X Mojave 10.14 inclusive
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
  } else {
    # Compatibility down to OS X Sierra 10.12 inclusive
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
  }

  LIBS += -F$${XPSDK_BASE}/Libraries/Mac -framework XPLM -framework XPWidgets
}

isEmpty(GIT_PATH) {
  GIT_REVISION=UNKNOWN
  GIT_REVISION_FULL=UNKNOWN
} else {
  GIT_REVISION=$$system('$$GIT_PATH' rev-parse --short HEAD)
  GIT_REVISION_FULL=$$system('$$GIT_PATH' rev-parse HEAD)
}

# Link all static in Windows to avoid DLL conflicts with other plugins
win32 {
  QMAKE_LFLAGS=-static -static-libstdc++ -static-libgcc
}

LIBS += -lz -L$$ATOOLS_LIB_PATH -latools
PRE_TARGETDEPS += $$ATOOLS_LIB_PATH/libatools.a
DEPENDPATH += $$ATOOLS_INC_PATH
INCLUDEPATH += $$PWD/src $$ATOOLS_INC_PATH $${XPSDK_BASE}/CHeaders/XPLM $${XPSDK_BASE}/CHeaders/Widgets
DEFINES += VERSION_NUMBER_LITTLEXPCONNECT='\\"$$VERSION_NUMBER\\"'
DEFINES += GIT_REVISION_LITTLEXPCONNECT='\\"$$GIT_REVISION\\"'
DEFINES += QT_NO_CAST_FROM_BYTEARRAY
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += XPLM302=1 XPLM301=1 XPLM300=1 XPLM210=1 XPLM200=1 APL=0 IBM=0 LIN=1

# Compiling the DLL but not using it
DEFINES += LITTLEXPCONNECT_LIBRARY

# =======================================================================
# Include build_options.pro with additional variables

exists($$PWD/../build_options.pro) {
   include($$PWD/../build_options.pro)

   !isEqual(QUIET, "true") {
     message($$PWD/../build_options.pro found.)
   }
} else {
   !isEqual(QUIET, "true") {
     message($$PWD/../build_options.pro not found.)
   }
}

# =======================================================================
# Print values when running qmake

!isEqual(QUIET, "true") {
message(-----------------------------------)
message(VERSION_NUMBER: $$VERSION_NUMBER)
message(GIT_REVISION: $$GIT_REVISION)
message(GIT_REVISION_FULL: $$GIT_REVISION_FULL)
message(GIT_PATH: $$GIT_PATH)
message(XPSDK_BASE: $$XPSDK_BASE)
message(ATOOLS_INC_PATH: $$ATOOLS_INC_PATH)
message(ATOOLS_LIB_PATH: $$ATOOLS_LIB_PATH)
message(DEPLOY_BASE: $$DEPLOY_BASE)
message(DEFINES: $$DEFINES)
message(INCLUDEPATH: $$INCLUDEPATH)
message(LIBS: $$LIBS)
message(TARGET_NAME: $$TARGET_NAME)
message(QT_INSTALL_PREFIX: $$[QT_INSTALL_PREFIX])
message(QT_INSTALL_LIBS: $$[QT_INSTALL_LIBS])
message(QT_INSTALL_PLUGINS: $$[QT_INSTALL_PLUGINS])
message(QT_INSTALL_TRANSLATIONS: $$[QT_INSTALL_TRANSLATIONS])
message(QT_INSTALL_BINS: $$[QT_INSTALL_BINS])
message(CONFIG: $$CONFIG)
message(-----------------------------------)
}

# =====================================================================
# Files

SOURCES += \
  src/main.cpp \
  src/xpconnect/aircraftfileloader.cpp \
  src/xpconnect/dataref.cpp \
  src/xpconnect/sharedmemorywriter.cpp \
  src/xpconnect/xpconnect.cpp \
  src/xpconnect/xplog.cpp \
  src/xpconnect/xpmenu.cpp

HEADERS += \
  src/littlexpconnect_global.h \
  src/xpconnect/aircraftfileloader.h \
  src/xpconnect/dataref.h \
  src/xpconnect/sharedmemorywriter.h \
  src/xpconnect/xpconnect.h \
  src/xpconnect/xplog.h \
  src/xpconnect/xpmenu.h

RESOURCES += \
    littlexpconnect.qrc

OTHER_FILES += \
  $$files(etc/*, true) \
  .travis.yml \
  .gitignore \
  BUILD.txt \
  CHANGELOG.txt \
  LICENSE.txt \
  README.txt \
  htmltidy.cfg \
  uncrustify.cfg

# =====================================================================
# Deployment commands
# =====================================================================

# Linux specific deploy target
unix:!macx {
  DEPLOY_DIR=\"$$DEPLOY_BASE/$$TARGET_NAME\"

  deploy.commands = rm -Rfv $${DEPLOY_DIR} &&
  deploy.commands += mkdir -pv $${DEPLOY_DIR}/64 &&
  deploy.commands += echo $$VERSION_NUMBER > $$DEPLOY_DIR/version.txt &&
  deploy.commands += echo $$GIT_REVISION_FULL > $$DEPLOY_DIR/revision.txt &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.so.1.0.0 $${DEPLOY_DIR}/64/lin.xpl &&
  deploy.commands += cp -vf $${PWD}/CHANGELOG.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/README.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/LICENSE.txt $${DEPLOY_DIR}
}


# MacOS specific deploy target
macx {
  DEPLOY_DIR=\"$$DEPLOY_BASE/$$TARGET_NAME\"

  deploy.commands = rm -Rfv $${DEPLOY_DIR} &&
  deploy.commands += mkdir -pv $${DEPLOY_DIR} &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.1.0.0.dylib $${DEPLOY_DIR}/mac.xpl &&
  deploy.commands += echo $$VERSION_NUMBER > $$DEPLOY_DIR/version.txt &&
  deploy.commands += echo $$GIT_REVISION_FULL > $$DEPLOY_DIR/revision.txt &&
  deploy.commands += cp -vf $${PWD}/CHANGELOG.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/README.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/LICENSE.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf \"$${PWD}/etc/Prepare Little Xpconnect.command\" $${DEPLOY_DIR} &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/QtCore.framework  $${DEPLOY_DIR} &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore.framework/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/QtCore_debug &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/QtCore_debug.prl &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore.framework/Versions/*/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/Versions/*/QtCore_debug &&
versionAtLeast(QT_VERSION, 6.0.0) {
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/QtCore5Compat.framework  $${DEPLOY_DIR} &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore5Compat.framework/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore5Compat.framework/QtCore_debug &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore5Compat.framework/QtCore_debug.prl &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore5Compat.framework/Versions/*/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore5Compat.framework/Versions/*/QtCore_debug &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/QtDBus.framework  $${DEPLOY_DIR} &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtDBus.framework/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtDBus.framework/QtCore_debug &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtDBus.framework/QtCore_debug.prl &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtDBus.framework/Versions/*/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtDBus.framework/Versions/*/QtCore_debug &&
}
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/QtGui.framework  $${DEPLOY_DIR} &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtGui.framework/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtGui.framework/QtGui_debug &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtGui.framework/QtGui_debug.prl &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtGui.framework/Versions/*/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtGui.framework/Versions/*/QtGui_debug
}

# Windows specific deploy target
win32 {
  defineReplace(p){return ($$shell_quote($$shell_path($$1)))}

  CONFIG(debug, debug|release):DLL_SUFFIX=d
  CONFIG(release, debug|release):DLL_SUFFIX=

  deploy.commands = rmdir /s /q $$p($$DEPLOY_BASE/$$TARGET_NAME) &
  deploy.commands += mkdir $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += mkdir $$p($$DEPLOY_BASE/$$TARGET_NAME/64) &&
  deploy.commands += echo $$VERSION_NUMBER > $$p($$DEPLOY_BASE/$$TARGET_NAME/version.txt) &&
  deploy.commands += echo $$GIT_REVISION_FULL > $$p($$DEPLOY_BASE/$$TARGET_NAME/revision.txt) &&
  deploy.commands += copy $$p($${OUT_PWD}/littlexpconnect.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME/64/win.xpl) &&
  deploy.commands += xcopy $$p($${PWD}/CHANGELOG.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy $$p($${PWD}/README.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy $$p($${PWD}/LICENSE.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME)
}

# =====================================================================
# Additional targets

# Need to copy data when compiling
all.depends = copydata

# Deploy needs compiling before
deploy.depends = all

QMAKE_EXTRA_TARGETS += deploy copydata all
