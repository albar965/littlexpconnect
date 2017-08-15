#-------------------------------------------------
#
# Project created by QtCreator 2017-08-13T16:49:54
#
#-------------------------------------------------

#QT       += network widgets
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
  QT_HOME=C:\\Qt\\5.9.1\\mingw53_32
  OPENSSL=C:\\OpenSSL-Win32
  GIT_BIN='C:\\Git\\bin\\git'
  XPSDK_HOME=C:\\X-PlaneSDK
  XP_HOME=C:\\Games\\X-Plane 11
}

# Linux ==================
unix:!macx {
  QT_HOME=/home/alex/Qt/5.9.1/gcc_64
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
  LIBS += -L $$PWD/../build-atools-$${CONF_TYPE}/$${CONF_TYPE} -l atools
  LIBS += -lz
  PRE_TARGETDEPS += $$PWD/../build-atools-$${CONF_TYPE}/$${CONF_TYPE}/libatools.a
  WINDEPLOY_FLAGS = --compiler-runtime
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


DEFINES += LITTLEXPCONNECT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# =====================================================================
# Files
# =====================================================================

# Get the current GIT revision to include it into the code
win32:DEFINES += GIT_REVISION='\\"$$system($${GIT_BIN} rev-parse --short HEAD)\\"'
unix:DEFINES += GIT_REVISION=
#'\\"$$system(git rev-parse --short HEAD)\\"'

SOURCES += \
    src/littlexpconnect.cpp \
    src/xpconnect.cpp \
    src/dataref.cpp

HEADERS += \
    src/littlexpconnect.h \
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

unix:!macx {
  DEPLOY_DIR=$${XP_HOME}/Resources/plugins/LittleXpConnect

  deploy.commands = mkdir -pv $${DEPLOY_DIR}/64 &&
  deploy.commands += cp -av $${OUT_PWD}/liblittlexpconnect.so.1.0.0 $${DEPLOY_DIR}/64/lin.xpl
}


QMAKE_EXTRA_TARGETS += deploy
