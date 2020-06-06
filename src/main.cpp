/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
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

#include "logging/logginghandler.h"
#include "logging/loggingutil.h"
#include "settings/settings.h"
#include "gui/consoleapplication.h"
#include "sharedmemorywriter.h"
#include "xpconnect.h"
#include "util/version.h"

#include <QDebug>

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
static const QLatin1Literal SETTINGS_OPTIONS_FETCH_RATE_MS("Options/FetchRate");
static const QLatin1Literal SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT("Options/FetchAiAircraft");
}

float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                         void *inRefcon);

/* Application object for event queue in server thread */
static atools::gui::ConsoleApplication *app = nullptr;

static bool fetchAi = true;
static float fetchRateSecs = 0.2f;

// Use a background thread to write the data to the shared memory to avoid simulator stutters due to
// locking
static SharedMemoryWriter *thread = nullptr;

/* Called on simulator startup */
PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO;

  // Register atools types so we can stream them
  qRegisterMetaType<atools::fs::sc::SimConnectData>();
  qRegisterMetaTypeStreamOperators<atools::geo::Pos>();

  // Create application object which is needed for the server thread event queue
  int argc = 0;
  app = new atools::gui::ConsoleApplication(argc, nullptr);
  app->setApplicationName("Little Xpconnect");
  app->setOrganizationName("ABarthel");
  app->setOrganizationDomain("littlenavmap.org");

  app->setApplicationVersion("1.0.19.develop"); // VERSION_NUMBER - Little Xpconnect

  // Initialize logging and force logfiles into the system or user temp directory
  LoggingHandler::initializeForTemp(Settings::getOverloadedPath(":/littlexpconnect/resources/config/logging.cfg"));
  LoggingUtil::logSystemInformation();
  LoggingUtil::logStandardPaths();

  // Pass plugin information to X-Plane
  QString info = QString("%1 %2").arg(app->applicationName()).arg(app->applicationVersion());
  atools::util::Version version(app->applicationVersion());

  // Program version and revision ==========================================
  if(version.isReleaseCandidate() || version.isBeta() || version.isDevelop())
    info += QString(" (%1)").arg(GIT_REVISION);

#ifndef QT_NO_DEBUG
  info += " - DEBUG";
#endif

  strcpy(outName, info.toLatin1().constData());
  strcpy(outSig, "ABarthel.LittleXpconnect.Connect");
  strcpy(outDesc, "Connects Little Navmap and Little Navconnect to X-Plane.");

  // Create an instance here since it will be accessed from the main server thread
  Settings::instance();

  Settings& settings = Settings::instance();
  fetchAi = settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool();
  fetchRateSecs = settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_FETCH_RATE_MS, 200).toFloat() / 1000.f;

  // Always successfull
  return 1;
}

/* Called when simulator terminates */
PLUGIN_API void XPluginStop(void)
{
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "sync settings";
  Settings::instance().syncSettings();

  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "Logging shutdown";
  LoggingHandler::shutdown();

  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "Logging shutdown done";
}

/* Enable plugin - can be called more than once during a simulator session */
PLUGIN_API int XPluginEnable(void)
{
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO;

  // Start the backgound writer
  thread = new SharedMemoryWriter();
  thread->start();

  // Register callback into method - first call in five seconds
  XPLMRegisterFlightLoopCallback(flightLoopCallback, 5.f, nullptr);

  return 1;
}

/* Disable plugin - can be called more than once during a simulator session */
PLUGIN_API void XPluginDisable()
{
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO;

  // Unregister call back
  XPLMUnregisterFlightLoopCallback(flightLoopCallback, nullptr);

  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "Terminating thread";
  thread->terminateThread();
  delete thread;
  thread = nullptr;
  qDebug() << "LittleXpconnect" << Q_FUNC_INFO << "Terminating thread done";
}

/* called on special messages like aircraft loaded, etc. */
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam)
{
  Q_UNUSED(inFromWho);
  Q_UNUSED(inMessage);
  Q_UNUSED(inParam);
}

float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter,
                         void *inRefcon)
{
  Q_UNUSED(inElapsedSinceLastCall);
  Q_UNUSED(inElapsedTimeSinceLastFlightLoop);
  Q_UNUSED(inCounter);
  Q_UNUSED(inRefcon);

  // Copy data from datarefs and pass it over to the thread for writing into the shared memory
  thread->fetchAndWriteData(fetchAi);

  return fetchRateSecs;
}
