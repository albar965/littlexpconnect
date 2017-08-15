/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
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

#include "xpconnect.h"

extern "C" {
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
}

#include "fs/sc/xpconnecthandler.h"
#include "fs/sc/simconnectdata.h"
#include "fs/sc/simconnectreply.h"
#include "gui/consoleapplication.h"

#include "fs/ns/navserver.h"
#include "fs/sc/connecthandler.h"
#include "fs/sc/datareaderthread.h"

#include "fs/sc/xpconnecthandler.h"

#include <QThread>

static XPLMDataRef dataRefLatDoubleDeg = NULL;
static XPLMDataRef dataRefLonDoubleDeg = NULL;
static XPLMDataRef dataRefSpeedFloatKias = NULL;
static XPLMDataRef dataRefElevDoubleMeter = NULL;

namespace xpc {

XpConnect *XpConnect::object = nullptr;

class ServerThread :
  public QThread
{
public:
  ServerThread()
  {

  }

  virtual ~ServerThread();

private:
  virtual void run() override;

};

ServerThread::~ServerThread()
{

}

void ServerThread::run()
{
  XpConnect::instance().createNavServer();
  exec();
  XpConnect::instance().destroyNavServer();
}

void XpConnect::createServerThread()
{
  deleteServerThread();
  serverThread = new ServerThread();
  serverThread->start();
}

void XpConnect::deleteServerThread()
{
  if(serverThread != nullptr)
  {
    serverThread->quit();
    serverThread->wait();
    delete serverThread;
    serverThread = nullptr;
  }
}

XpConnect::XpConnect()
{

}

XpConnect::~XpConnect()
{
  deleteServerThread();
}

XpConnect& XpConnect::instance()
{
  if(object == nullptr)
    object = new XpConnect();
  return *object;
}

void XpConnect::shutdown()
{
  delete object;
  object = nullptr;
}

void XpConnect::pluginEnable()
{
  dataRefLatDoubleDeg = XPLMFindDataRef("sim/flightmodel/position/latitude");

  dataRefLonDoubleDeg = XPLMFindDataRef("sim/flightmodel/position/longitude");

  dataRefSpeedFloatKias = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed");

  dataRefElevDoubleMeter = XPLMFindDataRef("sim/flightmodel/position/elevation");

  dataRefTrueHeading = XPLMFindDataRef("sim/flightmodel/position/true_psi");
  dataRefMagHeading = XPLMFindDataRef("sim/flightmodel/position/mag_psi");

  deleteServerThread();
  serverThread = new ServerThread();
  serverThread->start();
}

void XpConnect::pluginDisable()
{
  deleteServerThread();
}

float XpConnect::flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
                                    int inCounter)
{
  qDebug() << "LittleXpConnect" << Q_FUNC_INFO << "inElapsedSinceLastCall" << inElapsedSinceLastCall
           << "inElapsedTimeSinceLastFlightLoop" << inElapsedTimeSinceLastFlightLoop
           << "inCounter" << inCounter;

  {
    QMutexLocker locker(&copyDataMutex);
    currentData.userAircraft.indicatedSpeed = XPLMGetDataf(dataRefSpeedFloatKias);
    currentData.userAircraft.position = atools::geo::Pos(XPLMGetDatad(dataRefLonDoubleDeg),
                                                         XPLMGetDatad(dataRefLatDoubleDeg),
                                                         XPLMGetDatad(dataRefElevDoubleMeter));
  }

  return 1.f;
}

void XpConnect::receiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam)
{
  Q_UNUSED(inParam);
  qDebug() << "LittleXpConnect" << Q_FUNC_INFO << "inFromWho" << inFromWho << "inMessage" << inMessage;

  if(inFromWho == XPLM_PLUGIN_XPLANE)
  {
    switch(inMessage)
    {
      case XPLM_MSG_PLANE_CRASHED:
        qDebug() << "XPLM_MSG_PLANE_CRASHED";
        break;
      case XPLM_MSG_PLANE_LOADED:
        qDebug() << "XPLM_MSG_PLANE_LOADED";
        break;
      case XPLM_MSG_AIRPORT_LOADED:
        qDebug() << "XPLM_MSG_AIRPORT_LOADED";
        break;
      case XPLM_MSG_SCENERY_LOADED:
        qDebug() << "XPLM_MSG_SCENERY_LOADED";
        break;
      case XPLM_MSG_AIRPLANE_COUNT_CHANGED:
        qDebug() << "XPLM_MSG_AIRPLANE_COUNT_CHANGED";
        break;
      case XPLM_MSG_PLANE_UNLOADED: qDebug() << "XPLM_MSG_PLANE_UNLOADED";
        break;
    }
  }
}

bool XpConnect::copyData(atools::fs::sc::SimConnectData& data, int radiusKm, atools::fs::sc::Options options)
{
  Q_UNUSED(radiusKm);
  Q_UNUSED(options);

  {
    QMutexLocker locker(&copyDataMutex);
    if(currentData.isUserAircraftValid())
    {
      data = currentData;
      currentData = atools::fs::sc::EMPTY_SIMCONNECT_DATA;
    }
    else
      return false;
  }
  return true;
}

void XpConnect::createNavServer()
{
  qDebug() << "LittleXpConnect" << Q_FUNC_INFO;

  using namespace std::placeholders;
  atools::fs::sc::DataCopyFunctionType func = std::bind(&XpConnect::copyData, this, _1, _2, _3);

  bool verbose = false;
  atools::fs::sc::XpConnectHandler *xpHandler = new atools::fs::sc::XpConnectHandler(func, verbose);
  connectHandler = xpHandler;

  dataReader = new atools::fs::sc::DataReaderThread(nullptr, connectHandler, verbose);

  dataReader->setReconnectRateSec(10);
  dataReader->setUpdateRate(1000);

  // Create nav server but to not start it yet
  navServer = new atools::fs::ns::NavServer(nullptr, atools::fs::ns::NO_HTML, 51968);

  dataReader->start();
  navServer->startServer(dataReader);
}

void XpConnect::destroyNavServer()
{
  if(navServer != nullptr)
    navServer->stopServer();
  qDebug() << Q_FUNC_INFO << "navServer stopped";

  delete navServer;
  qDebug() << Q_FUNC_INFO << "navServer deleted";

  if(dataReader != nullptr)
    dataReader->terminateThread();
  qDebug() << Q_FUNC_INFO << "dataReader terminated";

  delete dataReader;
  qDebug() << Q_FUNC_INFO << "dataReader deleted";

  delete connectHandler;
  qDebug() << Q_FUNC_INFO << "connectHandler deleted";
}

} // namespace xpc
