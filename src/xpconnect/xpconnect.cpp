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

#include "xpconnect/xpconnect.h"
#include "xpconnect/dataref.h"
#include "xpconnect/xpdatarefs.h"

#include "aircraftfileloader.h"
#include "fs/sc/simconnectdata.h"
#include "fs/sc/simconnecttypes.h"
#include "fs/sc/simconnectuseraircraft.h"
#include "fs/util/fsutil.h"
#include "geo/calculations.h"

using atools::geo::kgToLbs;
using atools::geo::meterToFeet;
using atools::geo::meterToNm;
using atools::roundToInt;
using atools::geo::Pos;

namespace xpc {

XpConnect::XpConnect(bool verboseLogging)
  : verbose(verboseLogging)
{
  qDebug() << Q_FUNC_INFO;
  fileLoader = new AircraftFileLoader(verbose);
  fileLoader->setAircraftKeys({"acf/_name", "acf/_ICAO", "acf/_tailnum", "acf/_is_helicopter", "_engn/0/_type"});
}

XpConnect::~XpConnect()
{
  qDebug() << Q_FUNC_INFO;
  delete fileLoader;
  delete dataRefs;
}

bool XpConnect::fillSimConnectData(atools::fs::sc::SimConnectData& data, bool fetchAi, bool fetchAiAircraftInfo)
{
  atools::fs::sc::SimConnectUserAircraft& userAircraft = data.userAircraft;

  float actualAlt = meterToFeet(dataRefs->actualAltitudeMeter.valueFloat());
  userAircraft.position =
    Pos(dataRefs->lonPositionDeg.valueFloat(), dataRefs->latPositionDeg.valueFloat(), actualAlt);

  if(!userAircraft.position.isValid() || userAircraft.position.isNull())
    return false;

  userAircraft.magVarDeg = -dataRefs->magVarDeg.valueFloat();

  userAircraft.numberOfEngines = static_cast<quint8>(dataRefs->numberOfEngines.valueInt());

  // Wind and ambient parameters
  userAircraft.windSpeedKts = dataRefs->windSpeedKts.valueFloat();
  userAircraft.windDirectionDegT = dataRefs->windDirectionDegMag.valueFloat() + userAircraft.magVarDeg;
  userAircraft.ambientTemperatureCelsius = dataRefs->ambientTemperatureC.valueFloat();
  userAircraft.totalAirTemperatureCelsius = dataRefs->leTemperatureC.valueFloat();
  userAircraft.seaLevelPressureMbar = dataRefs->seaLevelPressurePascal.valueFloat() / 100.f;

  // Ice
  userAircraft.pitotIcePercent = static_cast<quint8>(dataRefs->pitotIcePercent.valueFloat() * 100.f);
  userAircraft.structuralIcePercent = static_cast<quint8>(std::max(dataRefs->structuralIcePercent.valueFloat(),
                                                                   dataRefs->structuralIcePercent2.valueFloat()) * 100.f);
  userAircraft.aoaIcePercent = static_cast<quint8>(std::max(dataRefs->aoaIcePercent.valueFloat(),
                                                            dataRefs->aoaIcePercent2.valueFloat()) * 100.f);
  userAircraft.inletIcePercent = static_cast<quint8>(dataRefs->inletIcePercent.valueFloat() * 100.f);
  userAircraft.propIcePercent = static_cast<quint8>(dataRefs->propIcePercent.valueFloat() * 100.f);
  userAircraft.statIcePercent = static_cast<quint8>(std::max(dataRefs->statIcePercent.valueFloat(),
                                                             dataRefs->statIcePercent2.valueFloat()) * 100.f);
  userAircraft.windowIcePercent = static_cast<quint8>(dataRefs->windowIcePercent.valueFloat() * 100.f);

  userAircraft.carbIcePercent = 0.f;
  FloatVector carbIce = dataRefs->carbIcePercent.valueFloatArr();
  for(int i = 0; i < carbIce.size() && i < userAircraft.numberOfEngines; i++)
    userAircraft.carbIcePercent = static_cast<quint8>(std::max(carbIce.value(i, 0.f) * 100.f,
                                                               static_cast<float>(userAircraft.carbIcePercent)));

  // Weight
  userAircraft.airplaneTotalWeightLbs = kgToLbs(dataRefs->airplaneTotalWeightKgs.valueFloat());
  userAircraft.airplaneMaxGrossWeightLbs = kgToLbs(dataRefs->airplaneMaxGrossWeightKgs.valueFloat());
  userAircraft.airplaneEmptyWeightLbs = kgToLbs(dataRefs->airplaneEmptyWeightKgs.valueFloat());

  // Fuel flow in weight
  userAircraft.fuelTotalWeightLbs = kgToLbs(dataRefs->fuelTotalWeightKgs.valueFloat());
  userAircraft.fuelFlowPPH = kgToLbs(dataRefs->fuelFlowKgSec8.valueFloatArrSum()) * 3600.f;

  userAircraft.ambientVisibilityMeter = dataRefs->isXplane12() ?
                                        atools::geo::nmToMeter(dataRefs->ambientVisibility.valueFloat()) :
                                        dataRefs->ambientVisibility.valueFloat();

  // Build local time and use timezone offset from simulator
  // X-Plane does not allow to set the year
  userAircraft.localDateTime = atools::correctDateLocal(dataRefs->localDateDays.valueInt() + 1, dataRefs->localTimeSec.valueFloat(),
                                                        dataRefs->zuluTimeSec.valueFloat(), userAircraft.position.getLonX());
  userAircraft.zuluDateTime = userAircraft.localDateTime.toUTC();

  // SimConnectAircraft
  userAircraft.airplaneTitle = dataRefs->airplaneTitle.valueString();
  userAircraft.airplaneModel = dataRefs->airplaneType.valueString();
  userAircraft.airplaneReg = dataRefs->airplaneTailnum.valueString();
  // userAircraft.airplaneType;           // not available - use model ICAO code in client
  // not available:
  // userAircraft.airplaneAirline; userAircraft.airplaneFlightnumber; userAircraft.fromIdent; userAircraft.toIdent;

  userAircraft.altitudeAboveGroundFt = meterToFeet(dataRefs->aglAltitudeMeter.valueFloat());
  userAircraft.groundAltitudeFt = actualAlt - userAircraft.altitudeAboveGroundFt;
  userAircraft.altitudeAutopilotFt = dataRefs->autopilotAltitudeFt.valueFloat();
  userAircraft.indicatedAltitudeFt = dataRefs->indicatedAltitudeFt.valueFloat();

  // Heading and track
  userAircraft.headingMagDeg = dataRefs->headingMagDeg.valueFloat();
  userAircraft.headingTrueDeg = dataRefs->headingTrueDeg.valueFloat();
  userAircraft.trackMagDeg = dataRefs->trackMagDeg.valueFloat();
  userAircraft.trackTrueDeg = userAircraft.trackMagDeg + userAircraft.magVarDeg;

  // Speed
  userAircraft.indicatedSpeedKts = dataRefs->indicatedSpeedKts.valueFloat();
  userAircraft.trueAirspeedKts = meterToNm(dataRefs->trueSpeedMs.valueFloat() * 3600.f);
  userAircraft.machSpeed = dataRefs->machSpeed.valueFloat();
  userAircraft.verticalSpeedFeetPerMin = dataRefs->verticalSpeedFpm.valueFloat();
  userAircraft.groundSpeedKts = meterToNm(dataRefs->groundSpeedMs.valueFloat() * 3600.f);

  // Get transponder code and Convert decimals to octal code
  userAircraft.transponderCode = atools::fs::util::decodeTransponderCode(dataRefs->transponderCode.valueInt());

  // Model
  // points to the tail of the aircraft
  userAircraft.modelRadiusFt = static_cast<quint16>(roundToInt(meterToFeet(dataRefs->aircraftSizeZ.valueFloat())));

  // points to the right side of the aircraft - wingspan will be used before model radius for painting
  userAircraft.wingSpanFt = static_cast<quint16>(roundToInt(meterToFeet(dataRefs->aircraftSizeX.valueFloat() * 2.)));

  atools::fs::sc::AircraftFlags simFlags = dataRefs->isXplane12() ? atools::fs::sc::SIM_XPLANE12 : atools::fs::sc::SIM_XPLANE11;

  // Set misc flags
  userAircraft.flags = atools::fs::sc::IS_USER | simFlags;
  userAircraft.flags.setFlag(atools::fs::sc::ON_GROUND, dataRefs->onGround.valueBool());
  userAircraft.flags.setFlag(atools::fs::sc::IN_RAIN, dataRefs->rainPercentage.valueFloat() > 0.1f);
  userAircraft.flags.setFlag(atools::fs::sc::SIM_PAUSED, dataRefs->simPaused.valueBool());
  userAircraft.flags.setFlag(atools::fs::sc::SIM_REPLAY, dataRefs->simReplay.valueBool());
  // IN_CLOUD = 0x0002, - not available
  // IN_SNOW = 0x0008,  - not available

  // Category is not available - Disable display of category on client
  userAircraft.category = atools::fs::sc::UNKNOWN;
  // AIRPLANE, HELICOPTER, BOAT, GROUNDVEHICLE, CONTROLTOWER, SIMPLEOBJECT, VIEWER

  // Value to calculate fuel volume from mass
  float fuelMassToVolDivider = 6.f;

  // Get the engine array
  IntVector engines = dataRefs->engineType8.valueIntArr();
  userAircraft.engineType = atools::fs::sc::UNSUPPORTED;
  // PISTON = 0, JET = 1, NO_ENGINE = 2, HELO_TURBINE = 3, UNSUPPORTED = 4, TURBOPROP = 5

  // Get engine type
  for(int i = 0; i < engines.size() && i < userAircraft.numberOfEngines; i++)
  {
    XpEngineType type = static_cast<XpEngineType>(engines.value(i, xpc::RECIP_CARB));
    switch(type)
    {
      case xpc::ELECTRIC:
      case xpc::RECIP_CARB:
      case xpc::RECIP_INJECTED:
        userAircraft.engineType = atools::fs::sc::PISTON;
        fuelMassToVolDivider = 6.f; // Avgas lbs to gallons at standard temp
        break;

      case xpc::FREE_TURBINE:
      case xpc::FIXED_TURBINE:
      case xpc::NEW_FREE_TURBINE:
      case xpc::NEW_FIXED_TURBINE:
        fuelMassToVolDivider = 6.7f; // JetA lbs to gallons at standard temp
        userAircraft.engineType = atools::fs::sc::TURBOPROP;
        break;

      case xpc::ROCKET:
      case xpc::TIP_ROCKETS:
      case xpc::LO_BYPASS_JET:
      case xpc::HI_BYPASS_JET:
        fuelMassToVolDivider = 6.7f; // JetA lbs to gallons at standard temp
        userAircraft.engineType = atools::fs::sc::JET;
        break;
    }
    if(userAircraft.engineType != atools::fs::sc::UNSUPPORTED)
      // Found one - stop here
      break;
  }

  // Calculate fuel volume based on type
  userAircraft.fuelTotalQuantityGallons = userAircraft.fuelTotalWeightLbs / fuelMassToVolDivider;
  userAircraft.fuelFlowGPH = userAircraft.fuelFlowPPH / fuelMassToVolDivider;

  // Load certain values from .acf file overriding dataref values
  fileLoader->loadAircraftFile(userAircraft, 0L);

  data.aiAircraft.clear();
  if(fetchAi)
  {
    quint32 objId = 1;

    // Carrier on first and frigate on second index in arrays
    FloatVector headings = dataRefs->boatHeadingDeg.valueFloatArr();
    FloatVector velocity = dataRefs->boatVelocityMsc.valueFloatArr();
    FloatVector x = dataRefs->boatXMtr.valueFloatArr();
    FloatVector y = dataRefs->boatYMtr.valueFloatArr();
    FloatVector z = dataRefs->boatZMtr.valueFloatArr();

    // Add aircraft carrier =============================================================
    if(headings.size() > 0 && velocity.size() > 0 && x.size() > 0 && y.size() > 0 && z.size() > 0)
    {
      const static int CARRIER_IDX = 0;
      atools::fs::sc::SimConnectAircraft carrier;
      carrier.deckHeight = static_cast<quint16>(atools::geo::meterToFeet(dataRefs->boatCarrierDeckHeightMtr.valueFloat()));
      carrier.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

      carrier.flags = simFlags;

      // Ground speed is null
      carrier.groundSpeedKts = atools::geo::meterPerSecToKnots(std::max(velocity.at(CARRIER_IDX), 0.f));
      if(!atools::inRange(0.1f, 70.f, carrier.groundSpeedKts))
        carrier.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;

      carrier.headingTrueDeg = headings.at(CARRIER_IDX);
      carrier.objectId = objId;
      carrier.category = atools::fs::sc::CARRIER;
      carrier.engineType = atools::fs::sc::UNSUPPORTED;
      carrier.position = localToWorld(x.at(CARRIER_IDX), y.at(CARRIER_IDX), z.at(CARRIER_IDX));
      carrier.position.setAltitude(atools::fs::sc::SC_INVALID_FLOAT);

      bool ok = true;
      ok &= carrier.position.isValidRange() && !carrier.position.isNull(0.01f);
      ok &= atools::inRange(0.f, 360.f, carrier.headingTrueDeg);
      ok &= atools::inRange(0, 100, int(carrier.deckHeight));

      if(ok)
      {
        data.aiAircraft.append(carrier);
        objId++;
      }
    }

    // Add frigate =============================================================
    if(headings.size() > 1 && velocity.size() > 1 && x.size() > 1 && y.size() > 1 && z.size() > 1)
    {
      const static int FRIGATE_IDX = 1;
      atools::fs::sc::SimConnectAircraft frigate;
      frigate.deckHeight = static_cast<quint16>(atools::geo::meterToFeet(dataRefs->boatFrigateDeckHeightMtr.valueFloat()));
      frigate.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

      frigate.flags = simFlags;
      frigate.groundSpeedKts = atools::geo::meterPerSecToKnots(std::max(velocity.at(FRIGATE_IDX), 0.f));
      if(!atools::inRange(0.1f, 70.f, frigate.groundSpeedKts))
        frigate.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.headingTrueDeg = headings.at(FRIGATE_IDX);
      frigate.objectId = objId;
      frigate.category = atools::fs::sc::FRIGATE;
      frigate.engineType = atools::fs::sc::UNSUPPORTED;
      frigate.position = localToWorld(x.at(FRIGATE_IDX), y.at(FRIGATE_IDX), z.at(FRIGATE_IDX));
      frigate.position.setAltitude(atools::fs::sc::SC_INVALID_FLOAT);

      bool ok = true;
      ok &= frigate.position.isValidRange() && !frigate.position.isNull(0.01f);
      ok &= atools::inRange(0.f, 360.f, frigate.headingTrueDeg);
      ok &= atools::inRange(0, 100, int(frigate.deckHeight));

      if(ok)
      {
        data.aiAircraft.append(frigate);
        objId++;
      }
    }

    // Count includes user aircraft
    int numTcasAircraft = dataRefs->tcasNumAcf.valueInt();
    bool hasTcasScheme = dataRefs->tcasModeCcode.isValid();
    QByteArray icaoTypeArr, flightIdArr;
    icaoTypeArr.resize(dataRefs->tcasIcaoType.sizeByteArr());
    flightIdArr.resize(dataRefs->tcasFlightId.sizeByteArr());

    // Get AI or multiplayer aircraft ===============================
    // Use TCAS scheme if there is at least one AI aircraft - ignore user at 0
    if(hasTcasScheme && numTcasAircraft > 1)
    {
      dataRefs->tcasIcaoType.valueByteArr(icaoTypeArr);
      dataRefs->tcasFlightId.valueByteArr(flightIdArr);

      // Use new TCAS scheme - index 0 is user - TCAS arrays also contain user ======================
      for(int i = 1; i < numTcasAircraft; i++)
      {
        Pos pos(dataRefs->tcasLon.valueFloatArr(i), dataRefs->tcasLat.valueFloatArr(i), meterToFeet(dataRefs->tcasEle.valueFloatArr(i)));
        if(pos.isValid() && !pos.isNull())
        {
          // Coordinates are ok too - must be an AI aircraft
          atools::fs::sc::SimConnectAircraft aircraft;
          aircraft.flags = simFlags;
          aircraft.position = pos;
          aircraft.headingTrueDeg = dataRefs->tcasPsi.valueFloatArr(i);
          aircraft.flags.setFlag(atools::fs::sc::ON_GROUND, dataRefs->tcasWeightOnWheels.valueBoolArr(i));

          aircraft.airplaneModel = QString(icaoTypeArr.mid(i * 8, 8));
          aircraft.airplaneReg = QString(flightIdArr.mid(i * 8, 8));

          // Mark fields as unavailable
          aircraft.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;

          // Ignore the vertical component
          aircraft.groundSpeedKts = atools::geo::meterPerSecToKnots(dataRefs->tcasVMsc.valueFloatArr(i));

          aircraft.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;

          aircraft.verticalSpeedFeetPerMin = dataRefs->tcasVerticalSpeed.valueFloatArr(i);

          // Get transponder code and Convert decimals to octal code
          aircraft.transponderCode = atools::fs::util::decodeTransponderCode(dataRefs->tcasModeCcode.valueIntArr(i));

          aircraft.objectId = objId;
          // aircraft.objectId = static_cast<quint32>(dataRefs->tcasModeSId.valueIntArr(i)) << 4; // Shift to have unique ids with ships

          aircraft.category = atools::fs::sc::AIRPLANE;
          aircraft.engineType = atools::fs::sc::UNSUPPORTED;

          if(fetchAiAircraftInfo)
            fileLoader->loadAircraftFile(aircraft, static_cast<quint32>(i));

          data.aiAircraft.append(aircraft);

          objId++;
        } // if(pos.isValid() && !pos.isNull())
      } // for(int i = 1; i < numTcasAircraft; i++)
    } // if(hasTcasScheme && numTcasAircraft > 1)
    else
    {
      // Use old multplayer scheme ============================================
      // Includes user aircraft - can return more than 20 despite providing only datarefs 1-19 (minus user)
      int numAi = std::min(getNumActiveAircraft(), 20) - 1;

      for(int i = 0; i < numAi; i++)
      {
        // Datarefs do not contain user
        const MultiplayerDataRefs& ref = dataRefs->multiplayerDataRefs.at(i);

        Pos pos(ref.lonPositionDegAi.valueFloat(), ref.latPositionDegAi.valueFloat(), meterToFeet(ref.actualAltitudeMeterAi.valueFloat()));

        if(pos.isValid() && !pos.isNull())
        {
          // Coordinates are ok too - must be an AI aircraft
          atools::fs::sc::SimConnectAircraft aircraft;
          aircraft.flags = simFlags;
          aircraft.position = pos;
          aircraft.headingTrueDeg = ref.headingTrueDegAi.valueFloat();
          aircraft.airplaneReg = ref.tailnum.valueString();

          // Mark fields as unavailable
          aircraft.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

          aircraft.objectId = objId;
          aircraft.category = atools::fs::sc::AIRPLANE;
          aircraft.engineType = atools::fs::sc::UNSUPPORTED;

          if(fetchAiAircraftInfo)
            fileLoader->loadAircraftFile(aircraft, static_cast<quint32>(i + 1));

          data.aiAircraft.append(aircraft);

          objId++;
        } // if(pos.isValid() && !pos.isNull())
      } // for(int i = 0; i < numAi; i++)
    } // if(numTcasAircraft > 0) ... else ...
  } // if(fetchAi)

  return true;
}

void XpConnect::initDataRefs()
{
  dataRefs = new XpDataRefs;
  dataRefs->init();
}

} // namespace xpc
