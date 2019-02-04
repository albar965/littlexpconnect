#*****************************************************************************
# Copyright 2015-2019 Alexander Barthel alex@littlenavmap.org
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

QT       += core

QT       -= gui
CONFIG += c++14
CONFIG += dll

TARGET = littlexpconnect
TEMPLATE = lib

# =======================================================================
# Adapt these paths for each operating system
# =======================================================================

CONFIG(debug, debug|release):CONF_TYPE=debug
CONFIG(release, debug|release):CONF_TYPE=release

DEFINES+=XPLM200=1
DEFINES+=APL=0
DEFINES+=IBM=0
DEFINES+=LIN=1

# Windows ==================
win32 {
  QT_HOME=C:\\msys64\\mingw64
  GIT_BIN='C:\\Git\\bin\\git'
  XPSDK_HOME='C:\\X-Plane SDK'
  XP_HOME='C:\\Games\\X-Plane 11'
}

# Linux ==================
unix:!macx {
  XPSDK_HOME=/home/alex/Programme/XPSDK
  XP_HOME=\"/home/alex/Daten/Programme/X-Plane 11\"
}

# Mac OS X ==================
macx {
  QT_HOME=/Users/alex/Qt/5.9.5/clang_64
  XPSDK_HOME=/Users/alex/XPSDK

  # Compatibility down to OS X 10.10
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}

# End of configuration section
# =======================================================================

DEFINES += QT_NO_CAST_FROM_BYTEARRAY
DEFINES += QT_NO_CAST_TO_ASCII
#DEFINES += QT_NO_CAST_FROM_ASCII

# Link all static in Windows to avoid DLL conflicts with other plugins
win32 {
QMAKE_LFLAGS=-static -static-libstdc++ -static-libgcc
}

# =====================================================================
# Dependencies
# =====================================================================

# Add dependencies to atools project and its static library to ensure relinking on changes
DEPENDPATH += $$PWD/../atools/src
INCLUDEPATH += $$PWD/../atools/src $$PWD/src $${XPSDK_HOME}/CHeaders/XPLM $${XPSDK_HOME}/CHeaders/Widgets

win32 {
DEFINES += _USE_MATH_DEFINES
  LIBS += -L$$PWD/../build-atools-$${CONF_TYPE}/$${CONF_TYPE} -latools
  LIBS += -L$${XPSDK_HOME}\Libraries\Win -lXPLM_64 -lXPWidgets_64
  LIBS += -lz
  PRE_TARGETDEPS += $$PWD/../build-atools-$${CONF_TYPE}/$${CONF_TYPE}/libatools.a
}

unix {
  LIBS += -L $$PWD/../build-atools-$${CONF_TYPE} -l atools
  PRE_TARGETDEPS += $$PWD/../build-atools-$${CONF_TYPE}/libatools.a

  # Use relative path to current .so directory to search for shared libraries
  QMAKE_RPATHDIR=.
}
unix:!macx {
  LIBS += -lz
}

macx {
  LIBS += -lz
  LIBS += -F$${XPSDK_HOME}/Libraries/Mac -framework XPLM -framework XPWidgets
}

# Compiling the DLL but not using it
DEFINES += LITTLEXPCONNECT_LIBRARY

# =====================================================================
# Files
# =====================================================================

# Get the current GIT revision to include it into the code
win32:DEFINES += GIT_REVISION='\\"$$system($${GIT_BIN} rev-parse --short HEAD)\\"'
unix:DEFINES += GIT_REVISION='\\"$$system(git rev-parse --short HEAD)\\"'

SOURCES += \
    src/xpconnect.cpp \
    src/dataref.cpp \
    src/main.cpp \
    src/sharedmemorywriter.cpp

HEADERS += \
    src/littlexpconnect_global.h \
    src/xpconnect.h \
    src/dataref.h \
    src/sharedmemorywriter.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    littlexpconnect.qrc

DISTFILES += \
    uncrustify.cfg \
    BUILD.txt \
    CHANGELOG.txt \
    htmltidy.cfg \
    LICENSE.txt \
    README.txt

# =====================================================================
# Deployment commands
# =====================================================================

# Linux specific deploy target
unix:!macx {
  DEPLOY_DIR=\"$$PWD/../deploy/Little Xpconnect\"

  deploy.commands = rm -Rfv $${DEPLOY_DIR} &&
  deploy.commands += mkdir -pv $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.so.1.0.0 $${DEPLOY_DIR}/64/lin.xpl &&
  deploy.commands += cp -av /usr/lib/x86_64-linux-gnu/libicui18n.so.* $${DEPLOY_DIR}/64/ &&
  deploy.commands += cp -av /usr/lib/x86_64-linux-gnu/libicuuc.so.* $${DEPLOY_DIR}/64/ &&
  deploy.commands += cp -av /usr/lib/x86_64-linux-gnu/libicudata.so.* $${DEPLOY_DIR}/64/ &&
  deploy.commands += cp -vf $${PWD}/CHANGELOG.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/README.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/LICENSE.txt $${DEPLOY_DIR}
}


# MacOS specific deploy target
macx {
  DEPLOY_DIR=\"$$PWD/../deploy/Little Xpconnect\"

  deploy.commands = rm -Rfv $${DEPLOY_DIR} &&
  deploy.commands += mkdir -pv $${DEPLOY_DIR} &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.1.0.0.dylib $${DEPLOY_DIR}/mac.xpl &&
  deploy.commands += cp -vf $${PWD}/CHANGELOG.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/README.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/LICENSE.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/QtCore.framework  $${DEPLOY_DIR} &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore.framework/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/QtCore_debug &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/QtCore_debug.prl &&
  deploy.commands += rm -Rfv $${DEPLOY_DIR}/QtCore.framework/Versions/*/Headers &&
  deploy.commands += rm -fv $${DEPLOY_DIR}/QtCore.framework/Versions/*/QtCore_debug
}

# Windows specific deploy target
win32 {
  # Create backslashed path
  WINPWD=$${PWD}
  WINPWD ~= s,/,\\,g
  WINOUT_PWD=$${OUT_PWD}
  WINOUT_PWD ~= s,/,\\,g
  DEPLOY_DIR_NAME=Little Xpconnect
  DEPLOY_DIR_WIN=\"$${WINPWD}\\..\\deploy\\$${DEPLOY_DIR_NAME}\"

  CONFIG(debug, debug|release):DLL_SUFFIX=d
  CONFIG(release, debug|release):DLL_SUFFIX=

  deploy.commands = rmdir /s /q $${DEPLOY_DIR_WIN} &
  deploy.commands += mkdir $${DEPLOY_DIR_WIN} &&
  deploy.commands += mkdir $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += copy $${WINOUT_PWD}\\$${CONF_TYPE}\\littlexpconnect.dll $${DEPLOY_DIR_WIN}\\64\\win.xpl &&
  deploy.commands += xcopy $${WINPWD}\\CHANGELOG.txt $${DEPLOY_DIR_WIN} &&
  deploy.commands += xcopy $${WINPWD}\\README.txt $${DEPLOY_DIR_WIN} &&
  deploy.commands += xcopy $${WINPWD}\\LICENSE.txt $${DEPLOY_DIR_WIN}
}

QMAKE_EXTRA_TARGETS += deploy
