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

#include "aircraftfileloader.h"
#include "dataref.h"

#include <QFile>

namespace xpc {

AircraftFileLoader::AircraftFileLoader()
{
}

AircraftFileLoader::~AircraftFileLoader()
{

}

void AircraftFileLoader::loadAcf(atools::fs::sc::SimConnectAircraft& aircraft, quint32 objId)
{
  // Read values from .acf file which are not available by the API ====================================
  QString aircraftModelFilepath = getAircraftModelFilepath(static_cast<int>(objId));
  AcfEntryType *keyValuePairs = acfFileValues.object(aircraftModelFilepath.toLower());
  if(keyValuePairs == nullptr)
  {
    // Read and cache the values
    keyValuePairs = new QHash<QString, QString>();

    // "acf/_is_airliner",  "acf/_is_general_aviation","acf/_callsign", "acf/_name", "acf/_descrip"
    readValuesFromAcfFile(*keyValuePairs, aircraftModelFilepath, acfKeys);

    acfFileValues.insert(aircraftModelFilepath.toLower(), keyValuePairs);
  }

  // Use attributes from the acf file ======================================
  // Cessna_172SP_seaplane.acf:P acf/_descrip Cessna 172 SP Skyhawk - 180HP
  // Cessna_172SP_seaplane.acf:P acf/_name Cessna Skyhawk (Floats)
  // L5_Sentinel.acf:P acf/_descrip Stinson L5 Sentinel - L5G with uprated engine to 235hp
  // L5_Sentinel.acf:P acf/_name Stinson L5 Sentinel
  // MD80.acf:P acf/_descrip MAD DOG
  // MD80.acf:P acf/_name MD-82  aircraft.airplaneTitle = keyValuePairs->value("acf/_name"); // Cessna 172 SP Skyhawk - 180HP
  fillAircraftValues(aircraft, keyValuePairs);
}

void AircraftFileLoader::readValuesFromAcfFile(AcfEntryType& keyValuePairs, const QString& filepath,
                                               const QStringList& keys) const
{
  static const QLatin1String PROPERTIES_END("PROPERTIES_END");
  static const QLatin1String PREFIX("P ");

  QFile file(filepath);

  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qDebug() << Q_FUNC_INFO << "Reading from" << filepath << "keys" << keys;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

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

void AircraftFileLoader::fillAircraftValues(atools::fs::sc::SimConnectAircraft& aircraft,
                                            const AcfEntryType *keyValuePairs) const
{
  aircraft.airplaneTitle = keyValuePairs->value("acf/_name");

  aircraft.airplaneModel = keyValuePairs->value("acf/_ICAO"); // C172

  if(aircraft.airplaneReg.isEmpty())
    aircraft.airplaneReg = keyValuePairs->value("acf/_tailnum"); // Registration N172SP

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
