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

/*
 * Handles the data fetching thread, the TCP server and dataref initialization and also contains the
 * callback which will fetch the data.
 *
 * Starts a main server thread with an active event loop that runs the TCP server and also starts the data fetcher thread.
 * The TCP server again starts a worker thread for each connection.
 *
 * The data reader sends signals to all TCP workers if a data package was read.
 *
 * Data is taken from datarefs by the periodically called flightLoopCallback.
 */
class XpConnect
{
public:
  ~XpConnect();

  /* Create and instance but does nothing else. Not thread safe. */
  static XpConnect& instance();

  /* delete instance an all servers and threads */
  void shutdown();

  /* Starts main server thread, all threads and the TCP server */
  void pluginEnable();

  /* Shuts everything down */
  void pluginDisable();

  /* Called by the XPLM API on the main thread context of X-Plane. Copies the data from datarefs quickly to
   *  a temp SimConnectData object currentData. */
  float flightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter);

  /* Plugin received a message. Called on the main thread context of X-Plane.*/
  void receiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam);

  /* Callback for the XpConnectHandler which locks the mutex and copies the data from the temp
   * SimConnectData object currentData. Called on the thread context of the DataReaderThread.
   *  returns true if something was copied. */
  bool copyData(atools::fs::sc::SimConnectData& data, int radiusKm, atools::fs::sc::Options options);

private:
  friend class ServerThread;

  XpConnect();

  /* Create and delete DataReaderThread and the TCP server */
  void createNavServer();
  void deleteNavServer();

  /* Create and delete the main event loop thread running all others */
  void createServerThread();
  void deleteServerThread();

  /* Initilaize the datarefs and print a warning if something is wrong. */
  void initDataRefs();

  /* Navserver that waits and accepts tcp connections. Starts a NavServerWorker in a thread for each connection */
  atools::fs::ns::NavServer *navServer = nullptr;

  /* Runs in background and fetches data from simulator - signals are sent to NavServerWorker threads */
  atools::fs::sc::DataReaderThread *dataReader = nullptr;

  /* Copies data and provides simulator status information */
  atools::fs::sc::ConnectHandler *connectHandler = nullptr;

  /* Main thread with event queue needed by the TCP server */
  ServerThread *serverThread = nullptr;

  /* Temp storage for the data copied from the datarefs to avoid unneded locking. */
  atools::fs::sc::SimConnectData currentData;

  /* Nedded to sync between flightLoopCallback on X-Plane main thread and copyData on DataReaderThread context. */
  QMutex copyDataMutex;

  /* Singleton instance */
  static XpConnect *object;
};

} // namespace xpc

#endif // LITTLEXPC_XPCONNECT_H
