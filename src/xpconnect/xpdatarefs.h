/*****************************************************************************
* Copyright 2015-2022 Alexander Barthel alex@littlenavmap.org
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

#ifndef XPDATAREFS_H
#define XPDATAREFS_H

#include "dataref.h"

namespace xpc {

enum XpEngineType
{
  // 0 = recip carb, 1 = recip injected, 2 = free turbine, 3 = electric, 4 = lo bypass jet, 5 = hi bypass jet, 6 = rocket, 7 = tip rockets, 8 = fixed turbine
  RECIP_CARB = 0,
  RECIP_INJECTED = 1,
  FREE_TURBINE = 2,
  ELECTRIC = 3,
  LO_BYPASS_JET = 4,
  HI_BYPASS_JET = 5,
  ROCKET = 6,
  TIP_ROCKETS = 7,
  FIXED_TURBINE = 8,

  /* New values since 11.30 */
  NEW_FREE_TURBINE = 9,
  NEW_FIXED_TURBINE = 10
};

// Datarefs for one AI or multiplayer aircraft
struct MultiplayerDataRefs
{
  DataRef headingTrueDegAi;
  DataRef latPositionDegAi;
  DataRef lonPositionDegAi;
  DataRef actualAltitudeMeterAi;
};

/*
 * Provides all datarefs needed to fill SimConnect aircraft.
 * Run in main thread only.
 */
class XpDataRefs
{
public:
  XpDataRefs()
  {
  }

  /* Do not allow copying */
  XpDataRefs(const XpDataRefs& other) = delete;
  XpDataRefs& operator=(const XpDataRefs& other) = delete;

  /* Initialize and find all datarefs */
  void init();

  bool isXplane12() const
  {
    return xplmVersion.valueInt() >= 120000;
  }

  /* Values documented in init() */
  DataRef xplmVersion, simPaused, simReplay, windSpeedKts, windDirectionDegMag, ambientTemperatureC, leTemperatureC, seaLevelPressurePascal,
          pitotIcePercent, structuralIcePercent, structuralIcePercent2, aoaIcePercent, aoaIcePercent2, inletIcePercent, propIcePercent,
          statIcePercent, statIcePercent2, windowIcePercent, carbIcePercent, airplaneTotalWeightKgs, airplaneMaxGrossWeightKgs,
          airplaneEmptyWeightKgs, airplanePayloadWeightKgs, fuelTotalWeightKgs, fuelFlowKgSec8, magVarDeg, ambientVisibilityM, trackMagDeg,
          localDateDays, localTimeSec, zuluTimeSec, airplaneTailnum, airplaneTitle, airplaneType, transponderCode, latPositionDeg,
          lonPositionDeg, indicatedSpeedKts, trueSpeedMs, groundSpeedMs, machSpeed, verticalSpeedFpm, indicatedAltitudeFt,
          actualAltitudeMeter, aglAltitudeMeter, autopilotAltitudeFt, headingTrueDeg, headingMagDeg, numberOfEngines, onGround,
          rainPercentage, aircraftSizeX, aircraftSizeZ, boatHeadingDeg, boatFrigateDeckHeightMtr, boatCarrierDeckHeightMtr, boatVelocityMsc,
          boatXMtr, boatYMtr, boatZMtr, engineType8;

  /* TCAS interface datarefs - all arrays of 64 elements */
  DataRef tcasNumAcf, tcasModeCcode, tcasLat, tcasLon, tcasEle, tcasVerticalSpeed, tcasVMsc, tcasPsi;

  /* Multiplayer aircraft from old (not TCAS) interface */
  QVector<MultiplayerDataRefs> multiplayerDataRefs;

private:
  // Contains all datarefs for simple initialization
  DataRefPtrVector dataRefs;

  // Contains multiplayer aircraft datarefs
  DataRefPtrVector aiDataRefs;
};

} // namespace xpc

#endif // XPDATAREFS_H
