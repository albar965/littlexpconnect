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

#ifndef LITTLEXPC_XPCONNECT_H
#define LITTLEXPC_XPCONNECT_H

#include "fs/sc/simconnecttypes.h"

extern "C" {
#include "XPLMDefs.h"
}

#include "fs/sc/simconnectdata.h"

#include <QMutex>

namespace atools {
namespace fs {
namespace ns {
class NavServer;
}
namespace sc {
class DataReaderThread;
class ConnectHandler;
}
}
}

namespace xpc {

class ServerThread;

class XpConnect
{
public:
  ~XpConnect();

  static XpConnect& instance();
  void shutdown();

  void pluginEnable();
  void pluginDisable();

  float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter);
  void receiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam);

  bool copyData(atools::fs::sc::SimConnectData& data, int radiusKm, atools::fs::sc::Options options);

private:
  friend class ServerThread;

  XpConnect();

  void createNavServer();
  void destroyNavServer();
  void createServerThread();
  void deleteServerThread();

  // Navserver that waits and accepts tcp connections. Starts a NavServerWorker in a thread for each connection.
  atools::fs::ns::NavServer *navServer = nullptr;

  // Runs in background and fetches data from simulator - signals are send to NavServerWorker threads
  atools::fs::sc::DataReaderThread *dataReader = nullptr;

  atools::fs::sc::ConnectHandler *connectHandler = nullptr;

  ServerThread *serverThread = nullptr;

  atools::fs::sc::SimConnectData currentData;

  QMutex copyDataMutex;
  static XpConnect *object;
};

} // namespace xpc

#endif // LITTLEXPC_XPCONNECT_H
