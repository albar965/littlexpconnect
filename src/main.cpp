/*****************************************************************************
* Copyright 2015-2024 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

// Include definitions for import and export for shared library
#include "littlexpconnect_global.h"

#include "gui/consoleapplication.h"
#include "xpconnect/xplog.h"
#include "logging/logginghandler.h"
#include "logging/loggingutil.h"
#include "settings/settings.h"
#include "util/version.h"
#include "xpconnect/sharedmemorywriter.h"
#include "xpconnect/xpmenu.h"

extern "C" {
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <QSharedMemory>
#include <QDataStream>
#include <QBuffer>
#include <QThread>
#include <QWaitCondition>
#include <QDir>
#include <QStringBuilder>

/*
 * This file contains the C functions needed by the XPLM API.
 *
 * This is the only file which exports symbols
 */

using atools::logging::LoggingHandler;
using atools::logging::LoggingUtil;
using atools::settings::Settings;

namespace lxc {
/* key names for atools::settings */
static const QLatin1String SETTINGS_OPTIONS_VERBOSE("Options/Verbose");
}

float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                         void *inRefcon);
void checkPath();

/* Application object for event queue in server thread */
static atools::gui::ConsoleApplication *app = nullptr;

static bool verbose = false;

// Use a background thread to write the data to the shared memory to avoid simulator stutters due to
// locking
static SharedMemoryWriter *thread = nullptr;
static XpMenu *menu = nullptr;

/* Called on simulator startup */
PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
  qDebug() << Q_FUNC_INFO << "Little Xpconnect";

  // Register atools types so we can stream them
  atools::fs::sc::registerMetaTypes();

  // Create application object which is needed for the server thread event queue
  int argc = 0;
  app = new atools::gui::ConsoleApplication(argc, nullptr);
#ifdef Q_OS_LINUX
  // Avoid various issues in X-Plane 12 like messed up colors ignored exclusion regions
  // See https://stackoverflow.com/questions/25661295/why-does-qcoreapplication-call-setlocalelc-all-by-default-on-unix-linux
  // https://github.com/albar965/littlenavmap/issues/983
  setlocale(LC_NUMERIC, "C");
#endif
  QCoreApplication::setApplicationName("Little Xpconnect");
  QCoreApplication::setOrganizationName("ABarthel");
  QCoreApplication::setOrganizationDomain("littlenavmap.org");

  QCoreApplication::setApplicationVersion(VERSION_NUMBER_LITTLEXPCONNECT);

  // Initialize logging and force logfiles into the system or user temp directory
  LoggingHandler::initializeForTemp(Settings::getOverloadedPath(":/littlexpconnect/resources/config/logging.cfg"));
  Settings::logMessages();
  LoggingUtil::logSystemInformation();
  LoggingUtil::logStandardPaths();

  // Pass plugin information to X-Plane
  QString info = QString("%1 %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
  atools::util::Version version(QCoreApplication::applicationVersion());

  // Program version and revision ==========================================
  if(version.isReleaseCandidate() || version.isBeta() || version.isDevelop())
    info += QString(" (%1)").arg(GIT_REVISION_LITTLEXPCONNECT);

#ifndef QT_NO_DEBUG
  info += " - DEBUG";
#endif

  // Initialize plugin parameters passed in to this function
  strcpy(outName, info.toLatin1().constData());
  strcpy(outSig, "ABarthel.LittleXpconnect.Connect");
  strcpy(outDesc, "Connects Little Navmap and Little Navconnect to X-Plane.");

  // Need to create an instance here since it will be accessed from the main server thread
  Settings& settings = Settings::instance();
  settings.remove("Options/FetchRate"); // Delete obsolete key in any case
  verbose = settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_VERBOSE, false).toBool();

  // Always successfull
  return 1;
}

/* Called when simulator terminates */
PLUGIN_API void XPluginStop(void)
{
  qDebug() << Q_FUNC_INFO << "Little Xpconnect";
}

/* Enable plugin - can be called more than once during a simulator session */
PLUGIN_API int XPluginEnable(void)
{
  qDebug() << Q_FUNC_INFO << "Little Xpconnect";

  // Start the backgound writer
  thread = new SharedMemoryWriter(verbose);
  thread->start();

  // Register callback into method - first call in five seconds
  XPLMRegisterFlightLoopCallback(flightLoopCallback, 5.f, nullptr);

  // Check installation path and print a warning to Log.txt if invalid
  checkPath();

  // Create menu structure and load values from settings
  menu = new XpMenu();
  menu->restoreState();
  menu->addMenu("Little Xpconnect");
  return 1;
}

/* Disable plugin - can be called more than once during a simulator session */
PLUGIN_API void XPluginDisable()
{
  qDebug() << Q_FUNC_INFO << "Little Xpconnect";

  menu->saveState();
  delete menu;
  menu = nullptr;
  // Unregister call back
  XPLMUnregisterFlightLoopCallback(flightLoopCallback, nullptr);

  qDebug() << Q_FUNC_INFO << "Little Xpconnect" << "Terminating thread";
  thread->terminateThread();
  delete thread;
  thread = nullptr;
  qDebug() << Q_FUNC_INFO << "Little Xpconnect" << "Terminating thread done";
}

/* called on special messages like aircraft loaded, etc. */
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam)
{
  Q_UNUSED(inFromWho)
  Q_UNUSED(inMessage)
  Q_UNUSED(inParam)
}

float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
  Q_UNUSED(inElapsedSinceLastCall)
  Q_UNUSED(inElapsedTimeSinceLastFlightLoop)
  Q_UNUSED(inCounter)
  Q_UNUSED(inRefcon)

  // Copy data from datarefs and pass it over to the thread for writing into the shared memory
  thread->fetchAndWriteData(menu->isFetchAi(), menu->isFetchAircraftInfo());

  // Return float seconds until next call
  return static_cast<float>(menu->getFetchRateMs()) / 1000.f;
}

void checkPath()
{
  // Get own id (int) and plugin path of DLL/.so
  XPLMPluginID pluginId = XPLMGetMyID();

  // Get path for xpl file - returns a colon separated path on macOS
  char xpPath[1024];
  memset(xpPath, '\0', 1024);
  XPLMGetPluginInfo(pluginId, nullptr, xpPath, nullptr, nullptr);

#if defined(Q_OS_MACOS)
  // Convert the stone-age macOS path notation from X-Plane - replace colons and add volumes to get access to disk
  // From "BigSur:Users:USER:Programme:X-Plane 11:Resources:plugins:Little Xpconnect:mac.xpl" to
  // "/Volumes/BigSur/Users/USER/Programme/X-Plane 11/Resources/plugins/Little Xpconnect/mac.xpl"
  QString path = "/Volumes/" + QString(xpPath).replace(':', '/');
#else
  QString path = QString(xpPath);
#endif

  xplog::logXpInfo(QString("Plugin id %1 installed in path \"%2\" (\"%3\"), app path \"%4\"").
                   arg(pluginId).arg(xpPath).arg(path).arg(QCoreApplication::applicationFilePath()));
  bool valid = true;

  // Check file extension
  QFileInfo pluginFile(path);
  valid &= pluginFile.suffix().compare("xpl", Qt::CaseInsensitive) == 0;

  // Installationp path
  // X-Plane/Resources/plugins/Little Xpconnect/mac.xpl
  // X-Plane/Resources/plugins/Little Xpconnect/64/win.xpl
  // X-Plane/Resources/plugins/Little Xpconnect/64/lin.xpl
  QDir pluginDir(pluginFile.absoluteDir());
#if !defined(Q_OS_MACOS)
  valid &= pluginDir.dirName() == "64";
  pluginDir.cdUp(); // Skip "64"
#endif
  pluginDir.cdUp(); // Skip "Little Xpconnect" or other sub-folder
  valid &= pluginDir.dirName().compare("plugins", Qt::CaseInsensitive) == 0;
  pluginDir.cdUp(); // Skip "plugins"
  valid &= pluginDir.dirName().compare("Resources", Qt::CaseInsensitive) == 0;

  if(!valid)
    xplog::logXpErr(QString("Plugin installed in the wrong path: \"%1\"").arg(pluginFile.absolutePath()));
  else
    xplog::logXpInfo(QString("Plugin path \"%1\" is ok").arg(pluginFile.absolutePath()));
}
