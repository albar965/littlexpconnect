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

#ifndef LITTLEXPC_XPCONNECT_H
#define LITTLEXPC_XPCONNECT_H

#include <QtGlobal>

namespace atools {
namespace fs {
namespace sc {
class SimConnectData;
class SimConnectAircraft;
}
}
}

namespace xpc {

class AircraftFileLoader;

/*
 * Class that has full access to SimConnectData.
 */
class XpConnect
{
public:
  XpConnect(bool verboseLogging);
  ~XpConnect();

  XpConnect(const XpConnect& other) = delete;
  XpConnect& operator=(const XpConnect& other) = delete;

  /* Fill SimConnectData from X-Plane datarefs. Returns true if data was found */
  bool fillSimConnectData(atools::fs::sc::SimConnectData& data, bool fetchAi);

  /* Initilaize the datarefs and print a warning if something is wrong. */
  void initDataRefs();

private:
  AircraftFileLoader *fileLoader;
  bool verbose = false;
};

} // namespace xpc

#endif // LITTLEXPC_XPCONNECT_H
