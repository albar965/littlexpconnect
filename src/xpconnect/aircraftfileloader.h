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

class QThreadPool;
class QMutex;

namespace xpc {

/*
 * Loads and caches required key entries from acf files to get information missing in the datarefs.
 * Read values from .acf file which are not available by the API.
 */
class AircraftFileLoader :
  public QObject
{
  Q_OBJECT

public:
  AircraftFileLoader(bool verboseLogging);
  virtual ~AircraftFileLoader() override;

  AircraftFileLoader(const AircraftFileLoader& other) = delete;
  AircraftFileLoader& operator=(const AircraftFileLoader& other) = delete;

  /* Load and cache required entries from acf file for given aircraft id. Result is stored in aircraft */
  void loadAircraftFile(atools::fs::sc::SimConnectAircraft& aircraft, quint32 objId);

  /* Set keys to read from files.
   * Keys minus prefix "P " like "acf/_name", "acf/_ICAO" */
  void setAircraftKeys(const QStringList& value)
  {
    aircraftKeys = value;
  }

  const QStringList& getAircraftKeys() const
  {
    return aircraftKeys;
  }

private:
  typedef QHash<QString, QString> AircraftEntryType;

  /* Read keys from acf file. Reading stops if all keys are found. Use rarely and cache values since
   * it can read up to 100000 lines of text.
   * Runs in thread context.*/
  static void readValuesFromAircraftFile(AircraftEntryType& keyValuePairs, const QString& filepath,
                                         const QStringList& keys, bool verboseLogging);

  /* Fill and decode keys into aircraft */
  static void fillAircraftValues(atools::fs::sc::SimConnectAircraft& aircraft, const AircraftEntryType *keyValuePairs);

  /* Started by QtConcurrent::run() in a separate thread and accesses the cache. Runs in thread context. */
  void loadKeysRunner(QString aircraftModelFilepath, QStringList keys);

  /* Cache key value pairs from acf files to avoid reading the files. Empty entries indicate file not found. */
  QCache<QString, AircraftEntryType> *aircraftFileCache;
  QMutex *aircraftFileValuesMutex;

  /* List of acf files currently loading. Key is lower case filepath. */
  QSet<QString> aircraftFileKeysLoading;
  QMutex *aircraftFileKeysLoadingMutex;

  QThreadPool *threadPool;

  QStringList aircraftKeys;
  bool verbose = false;
};

} // namespace xpc

#endif // XPCONNECT_AIRCRAFTFILELOADER_H
