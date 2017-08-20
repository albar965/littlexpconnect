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

namespace atools {
namespace fs {
namespace sc {
class SimConnectData;
}
}
}

namespace xpc {

/*
 * Class that has full access to SimConnectData.
 */
class XpConnect
{
public:
  /* Fill SimConnectData from X-Plane datarefs. Returns true if data was found */
  static bool fillSimConnectData(atools::fs::sc::SimConnectData& data, bool fetchAi);

  /* Initilaize the datarefs and print a warning if something is wrong. */
  static void initDataRefs();

};

} // namespace xpc

#endif // LITTLEXPC_XPCONNECT_H
