#-------------------------------------------------
#
# Project created by QtCreator 2017-08-13T16:49:54
#
#-------------------------------------------------

QT       += core network

QT       -= gui
CONFIG += c++11

TARGET = littlexpconnect
TEMPLATE = lib

# =======================================================================
# Adapt these paths for each operating system
# =======================================================================

CONFIG(debug, debug|release):CONF_TYPE=debug
CONFIG(release, debug|release):CONF_TYPE=release

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
}

# End of configuration section
# =======================================================================

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
}
unix:!macx {
  LIBS += -lz
}

macx {
  LIBS += -lz
}

# Compiling the DLL but not using it
DEFINES += LITTLEXPCONNECT_LIBRARY

# =====================================================================
# Files
# =====================================================================

# Get the current GIT revision to include it into the code
win32:DEFINES += GIT_REVISION='\\"$$system($${GIT_BIN} rev-parse --short HEAD)\\"'
unix:DEFINES += GIT_REVISION=
#'\\"$$system(git rev-parse --short HEAD)\\"'

SOURCES += \
    src/xpconnect.cpp \
    src/dataref.cpp \
    src/main.cpp

HEADERS += \
    src/littlexpconnect_global.h \
    src/xpconnect.h \
    src/dataref.h

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
  DEPLOY_DIR=\"$$PWD/../deploy/Little XpConnect\"

  deploy.commands = rm -Rfv $${DEPLOY_DIR} &&
  deploy.commands = mkdir -pv $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.so.1.0.0 $${DEPLOY_DIR}/64/lin.xpl &&
  deploy.commands += cp -vf $${PWD}/CHANGELOG.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/README.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vf $${PWD}/LICENSE.txt $${DEPLOY_DIR} &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/libicudata.so*  $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/libicui18n.so*  $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/libicuuc.so*  $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/libQt5Core.so*  $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -vfa $${QT_HOME}/lib/libQt5Network.so*  $${DEPLOY_DIR}/64
}


# Windows specific deploy target
win32 {
  # Create backslashed path
  WINPWD=$${PWD}
  WINPWD ~= s,/,\\,g
  WINOUT_PWD=$${OUT_PWD}
  WINOUT_PWD ~= s,/,\\,g
  DEPLOY_DIR_NAME=Little XpConnect
  DEPLOY_DIR_WIN=\"$${WINPWD}\\..\\deploy\\$${DEPLOY_DIR_NAME}\"

  CONFIG(debug, debug|release):DLL_SUFFIX=d
  CONFIG(release, debug|release):DLL_SUFFIX=

  deploy.commands = rmdir /s /q $${DEPLOY_DIR_WIN} &
  deploy.commands += mkdir $${DEPLOY_DIR_WIN} &&
  deploy.commands += mkdir $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += mkdir $${DEPLOY_DIR_WIN}\\64\\platforms &&
  deploy.commands += copy $${WINOUT_PWD}\\$${CONF_TYPE}\\littlexpconnect.dll $${DEPLOY_DIR_WIN}\\64\\win.xpl &&
  deploy.commands += xcopy $${WINPWD}\\CHANGELOG.txt $${DEPLOY_DIR_WIN} &&
  deploy.commands += xcopy $${WINPWD}\\README.txt $${DEPLOY_DIR_WIN} &&
  deploy.commands += xcopy $${WINPWD}\\LICENSE.txt $${DEPLOY_DIR_WIN} &&
  deploy.commands += xcopy $${QT_HOME}\\share\\qt5\\plugins\\platforms\qwindows.dll $${DEPLOY_DIR_WIN}\\64\\platforms &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libgcc*.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libstdc*.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libwinpthread*.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libicudt$${DLL_SUFFIX}58.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libicuin$${DLL_SUFFIX}58.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\libicuuc$${DLL_SUFFIX}58.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\zlib1.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\Qt5Network$${DLL_SUFFIX}.dll $${DEPLOY_DIR_WIN}\\64 &&
  deploy.commands += xcopy $${QT_HOME}\\bin\\Qt5Core$${DLL_SUFFIX}.dll $${DEPLOY_DIR_WIN}\\64
}

QMAKE_EXTRA_TARGETS += deploy
