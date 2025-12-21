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

#include "xpconnect/aircraftfileloader.h"
#include "xpconnect/dataref.h"
#include "atools.h"

#include <QFile>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>

namespace xpc {

AircraftFileLoader::AircraftFileLoader(bool verboseLogging)
  : verbose(verboseLogging)
{
  aircraftFileCache = new QCache<QString, AircraftEntryType>;

  // Use a thread pool limited to one thread to avoid putting too much load on the simulator
  threadPool = new QThreadPool(this);
  threadPool->setMaxThreadCount(1);

  // Protect fields accessed by thread
  aircraftFileValuesMutex = new QMutex;
  aircraftFileKeysLoadingMutex = new QMutex;
}

AircraftFileLoader::~AircraftFileLoader()
{
  // Destroys the QThreadPool. This function will block until all runnables have been completed.
  delete threadPool;

  delete aircraftFileCache;

  delete aircraftFileValuesMutex;
  delete aircraftFileKeysLoadingMutex;
}

void AircraftFileLoader::loadKeysRunner(QString aircraftModelFilepath, QStringList keys)
{
  // Runs in separate thread
  if(verbose)
    qDebug() << Q_FUNC_INFO << "Entry" << aircraftModelFilepath;

  // Read and cache the values
  AircraftEntryType *keyValuePairs = new QHash<QString, QString>();
  QString aircraftModelKey = aircraftModelFilepath.toLower();

  // "acf/_is_airliner",  "acf/_is_general_aviation","acf/_callsign", "acf/_name", "acf/_descrip"
  readValuesFromAircraftFile(*keyValuePairs, aircraftModelFilepath, keys, verbose);

  {
    // Add to cache
    QMutexLocker locker(aircraftFileValuesMutex);
    aircraftFileCache->insert(aircraftModelKey, keyValuePairs);
  }

  {
    // Not loading anymore - remove from set
    QMutexLocker locker(aircraftFileKeysLoadingMutex);
    aircraftFileKeysLoading.remove(aircraftModelKey);
  }

  if(verbose)
    qDebug() << Q_FUNC_INFO << "Exit" << aircraftModelFilepath;
}

void AircraftFileLoader::loadAircraftFile(atools::fs::sc::SimConnectAircraft& aircraft, quint32 objId)
{
  if(aircraftIdsNotFound.contains(objId))
    // File does not exist for this cached id
    return;

  QString aircraftModelFilepath = getAircraftModelFilepath(static_cast<int>(objId));
  QString aircraftModelKey = aircraftModelFilepath.toLower();
  AircraftEntryType keyValuePairs;
  bool found = false;

  if(!aircraftModelKey.isEmpty())
  {
    // Key is ok - look for entry in cache
    QMutexLocker locker(aircraftFileValuesMutex);
    AircraftEntryType *keyValuePairsPtr = aircraftFileCache->object(aircraftModelKey);
    if(keyValuePairsPtr != nullptr)
    {
      // Found - create a copy and fill later after leaving lock
      keyValuePairs = *keyValuePairsPtr;
      found = true;
    }
  }

  if(found)
    // Use cached and copied attributes from the acf file ======================================
    // Cessna_172SP_seaplane.acf:P acf/_descrip Cessna 172 SP Skyhawk - 180HP
    // Cessna_172SP_seaplane.acf:P acf/_name Cessna Skyhawk (Floats)
    // L5_Sentinel.acf:P acf/_descrip Stinson L5 Sentinel - L5G with uprated engine to 235hp
    // L5_Sentinel.acf:P acf/_name Stinson L5 Sentinel
    // MD80.acf:P acf/_descrip MAD DOG
    // MD80.acf:P acf/_name MD-82
    fillAircraftValues(aircraft, &keyValuePairs);
  else
  {
    // Not in cache ... ============
    // Check if file is valid and exists - otherwise add objId to negative cache
    if(aircraftModelFilepath.isEmpty() || !QFile::exists(aircraftModelFilepath))
      aircraftIdsNotFound.insert(objId);
    else
    {
      // Not found in cache
      QMutexLocker locker(aircraftFileKeysLoadingMutex);

      // Is this already loading in background thread?
      if(!aircraftFileKeysLoading.contains(aircraftModelKey))
      {
        // Remember key which is loading now - thread will remove this key on completion
        aircraftFileKeysLoading.insert(aircraftModelKey);

        // Threadpool will wait until a free thread is available
        auto result = QtConcurrent::run(threadPool, &AircraftFileLoader::loadKeysRunner, this, aircraftModelFilepath, aircraftKeys);
      }
    }
  }
}

void AircraftFileLoader::readValuesFromAircraftFile(AircraftEntryType& keyValuePairs, const QString& filepath,
                                                    const QStringList& keys, bool verboseLogging)
{
  static const QLatin1String PROPERTIES_END("PROPERTIES_END");
  static const QLatin1String PREFIX("P ");

  // Check if file is valid, readable, has size > 0, etc.
  if(!atools::checkFile(Q_FUNC_INFO, QFileInfo(filepath), verboseLogging))
    return;

  QFile file(filepath);
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qDebug() << Q_FUNC_INFO << "Reading from" << filepath << "keys" << keys;

    QTextStream stream(&file);
    int lines = 0;
    // Read until all keys are found or at end of file
    while(!stream.atEnd() && keyValuePairs.size() < keys.size())
    {
      QString line = stream.readLine().trimmed();

      if(line == PROPERTIES_END)
        break;

      if(!line.startsWith(PREFIX))
        continue;

      QString key = line.section(' ', 1, 1);

      if(keys.contains(key))
        // Found a required key
        keyValuePairs.insert(key, line.section(' ', 2).trimmed());
      lines++;
    }

    file.close();
    qDebug() << Q_FUNC_INFO << "Reading from" << filepath << "done read" << lines << "lines"
             << "Key/values found" << keyValuePairs;
  }
  else
    qWarning() << Q_FUNC_INFO << "Cannot open file" << filepath << "error" << file.errorString();
}

void AircraftFileLoader::fillAircraftValues(atools::fs::sc::SimConnectAircraft& aircraft, const AircraftEntryType *keyValuePairs)
{
  if(aircraft.airplaneTitle.isEmpty())
    aircraft.airplaneTitle = keyValuePairs->value("acf/_name");

  QString model = keyValuePairs->value("acf/_ICAO");
  if(aircraft.airplaneModel.isEmpty())
    aircraft.airplaneModel = model; // C172
  else if(!model.isEmpty() && aircraft.airplaneModel != model && verbose)
    qWarning() << Q_FUNC_INFO << "Aircraft type mismatch" << aircraft.airplaneModel << model;

  QString reg = keyValuePairs->value("acf/_tailnum");
  if(aircraft.airplaneReg.isEmpty())
    aircraft.airplaneReg = reg; // Registration N172SP
  else if(!reg.isEmpty() && aircraft.airplaneReg != reg && verbose)
    qWarning() << Q_FUNC_INFO << "Aircraft reg mismatch" << aircraft.airplaneReg << reg;

  // Engine type - use first engine only ======================
  // PISTON = 0, JET = 1, NO_ENGINE = 2, HELO_TURBINE = 3, UNSUPPORTED = 4, TURBOPROP = 5
  aircraft.engineType = atools::fs::sc::UNSUPPORTED;
  const QString engineType = keyValuePairs->value("_engn/0/_type");
  if(engineType.startsWith("JET") || engineType.startsWith("ROC"))
    aircraft.engineType = atools::fs::sc::JET;
  else if(engineType.startsWith("RCP"))
    aircraft.engineType = atools::fs::sc::PISTON;
  else if(engineType.startsWith("TRB"))
    aircraft.engineType = atools::fs::sc::TURBOPROP;

  // Extra Aircraft/B-52G NASA/B-52G NASA.acf:P _engn/0/_type JET
  // Extra Aircraft/B747-100 NASA/B747-100 NASA.acf:P _engn/0/_type JET_HIB
  // Extra Aircraft/C-130/C-130.acf:P _engn/0/_type TRB_FIX
  // Extra Aircraft/X-15/X-15.acf:P _engn/0/_type ROC
  // Laminar Research/Aerolite 103/Aerolite_103.acf:P _engn/0/_type RCP_CRB
  // Laminar Research/Baron B58/Baron_58.acf:P _engn/0/_type RCP_INJ
  // Laminar Research/Boeing B747-400/747-400.acf:P _engn/0/_type JET
  // Laminar Research/KingAir C90B/C90B.acf:P _engn/0/_type TRB_FRE

  // Category ======================
  // AIRPLANE, HELICOPTER, BOAT, GROUNDVEHICLE, CONTROLTOWER, SIMPLEOBJECT, VIEWER, UNKNOWN
  if(keyValuePairs->value("acf/_is_helicopter").toInt() == 1)
    aircraft.category = atools::fs::sc::HELICOPTER;
  else
    aircraft.category = atools::fs::sc::AIRPLANE;
}

} // namespace xpc
