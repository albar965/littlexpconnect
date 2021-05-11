/*****************************************************************************
* Copyright 2015-2021 Alexander Barthel alex@littlenavmap.org
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

#ifndef XPCONNECT_AIRCRAFTFILELOADER_H
#define XPCONNECT_AIRCRAFTFILELOADER_H

#include "fs/sc/simconnectaircraft.h"

#include <QCache>

namespace xpc {

/*
 * Loads and caches required key entries from acf files to get information missing in the datarefs.
 */
class AircraftFileLoader :
  public QObject
{
  Q_OBJECT

public:
  AircraftFileLoader();
  virtual ~AircraftFileLoader() override;

  /* Load and cache required entries from acf file for given aircraft id. Result is stored in aircraft */
  void loadAcf(atools::fs::sc::SimConnectAircraft& aircraft, quint32 objId);

  const QStringList& getAcfKeys() const
  {
    return acfKeys;
  }

  /* Set keys to read from files.
   * Keys minus prefix "P " like "acf/_name", "acf/_ICAO" */
  void setAcfKeys(const QStringList& value)
  {
    acfKeys = value;
  }

private:
  typedef QHash<QString, QString> AcfEntryType;

  /* Read keys from acf file. Reading stops if all keys are found. Use rarely and cache values since
   * it can read up to 100000 lines of text. */
  void readValuesFromAcfFile(AcfEntryType& keyValuePairs, const QString& filepath, const QStringList& keys) const;

  void fillAircraftValues(atools::fs::sc::SimConnectAircraft& aircraft, const AcfEntryType *keyValuePairs) const;

  /* Cache key value pairs from acf files to avoid reading the files. Empty entries indicate file not found. */
  QCache<QString, AcfEntryType> acfFileValues;

  QStringList acfKeys;

};

} // namespace xpc

#endif // XPCONNECT_AIRCRAFTFILELOADER_H
