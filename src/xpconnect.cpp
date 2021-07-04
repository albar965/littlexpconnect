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

#include "xpconnect.h"
#include "dataref.h"

#include "fs/sc/simconnectuseraircraft.h"
#include "fs/sc/simconnectdata.h"
#include "fs/sc/simconnecttypes.h"
#include "geo/calculations.h"
#include "fs/util/fsutil.h"
#include "aircraftfileloader.h"

using atools::geo::kgToLbs;
using atools::geo::meterToFeet;
using atools::geo::meterToNm;
using atools::roundToInt;
using atools::geo::Pos;

namespace xpc {

// DataRefs ======================================================
namespace dr {
// Contains all datarefs for simple initialization
static DataRefPtrVector dataRefs;

// Contains multiplayer aircraft datarefs
static DataRefPtrVector aiDataRefs;

static DataRef simBuild(dataRefs, "sim/version/sim_build_string");
static DataRef xplmBuild(dataRefs, "sim/version/xplm_build_string");
static DataRef simPaused(dataRefs, "sim/time/paused");
static DataRef simReplay(dataRefs, "sim/operation/prefs/replay_mode");

// SimConnectUserAircraft
static DataRef windSpeedKts(dataRefs, "sim/cockpit2/gauges/indicators/wind_speed_kts");
static DataRef windDirectionDegMag(dataRefs, "sim/cockpit2/gauges/indicators/wind_heading_deg_mag");

// Temperatures
// Static air temperature (SAT) is also called: outside air temperature (OAT) or true air temperature
// Lower than TAT
static DataRef ambientTemperatureC(dataRefs, "sim/weather/temperature_ambient_c");

// Total air temperature (TAT) is also called: indicated air temperature (IAT) or ram air temperature (RAT)
// higher than SAT
static DataRef leTemperatureC(dataRefs, "sim/weather/temperature_le_c");

static DataRef seaLevelPressurePascal(dataRefs, "sim/physics/earth_pressure_p");

// Ice
static DataRef pitotIcePercent(dataRefs, "sim/flightmodel/failures/pitot_ice");
static DataRef structuralIcePercent(dataRefs, "sim/flightmodel/failures/frm_ice");
static DataRef structuralIcePercent2(dataRefs, "sim/flightmodel/failures/frm_ice2");
static DataRef aoaIcePercent(dataRefs, "sim/flightmodel/failures/aoa_ice");
static DataRef aoaIcePercent2(dataRefs, "sim/flightmodel/failures/aoa_ice2");
static DataRef inletIcePercent(dataRefs, "sim/flightmodel/failures/inlet_ice");
static DataRef propIcePercent(dataRefs, "sim/flightmodel/failures/prop_ice");
static DataRef statIcePercent(dataRefs, "sim/flightmodel/failures/stat_ice");
static DataRef statIcePercent2(dataRefs, "sim/flightmodel/failures/stat_ice2");
static DataRef windowIcePercent(dataRefs, "sim/flightmodel/failures/window_ice");
static DataRef carbIcePercent(dataRefs, "sim/flightmodel/engine/ENGN_crbice");

// Weight
static DataRef airplaneTotalWeightKgs(dataRefs, "sim/flightmodel/weight/m_total");
static DataRef airplaneMaxGrossWeightKgs(dataRefs, "sim/aircraft/weight/acf_m_max");
static DataRef airplaneEmptyWeightKgs(dataRefs, "sim/aircraft/weight/acf_m_empty");
static DataRef airplanePayloadWeightKgs(dataRefs, "sim/flightmodel/weight/m_fixed");

// static DataRef fuelTotalQuantityGallons(dataRefs, ""); value calculated
static DataRef fuelTotalWeightKgs(dataRefs, "sim/flightmodel/weight/m_fuel_total");

// Value for up to eight engines
static DataRef fuelFlowKgSec8(dataRefs, "sim/cockpit2/engine/indicators/fuel_flow_kg_sec");
// static DataRef fuelFlowGPH(dataRefs, ""); value calculated

static DataRef magVarDeg(dataRefs, "sim/flightmodel/position/magnetic_variation");
static DataRef ambientVisibilityM(dataRefs, "sim/weather/visibility_reported_m");
static DataRef trackMagDeg(dataRefs, "sim/cockpit2/gauges/indicators/ground_track_mag_pilot");
// static DataRef trackTrueDeg(dataRefs, ""); value calculated

// Date and time
static DataRef localDateDays(dataRefs, "sim/time/local_date_days");
static DataRef localTimeSec(dataRefs, "sim/time/local_time_sec");
static DataRef zuluTimeSec(dataRefs, "sim/time/zulu_time_sec");

// SimConnectAircraft
static DataRef airplaneTailnum(dataRefs, "sim/aircraft/view/acf_tailnum");
static DataRef airplaneTitle(dataRefs, "sim/aircraft/view/acf_descrip");
static DataRef airplaneType(dataRefs, "sim/aircraft/view/acf_ICAO");
static DataRef transponderCode(dataRefs, "sim/cockpit/radios/transponder_code");

// Position
static DataRef latPositionDeg(dataRefs, "sim/flightmodel/position/latitude");
static DataRef lonPositionDeg(dataRefs, "sim/flightmodel/position/longitude");

// Speeds
static DataRef indicatedSpeedKts(dataRefs, "sim/flightmodel/position/indicated_airspeed");
static DataRef trueSpeedMs(dataRefs, "sim/flightmodel/position/true_airspeed");
static DataRef groundSpeedMs(dataRefs, "sim/flightmodel/position/groundspeed");
static DataRef machSpeed(dataRefs, "sim/flightmodel/misc/machno");
static DataRef verticalSpeedFpm(dataRefs, "sim/flightmodel/position/vh_ind_fpm");

// Altitude
static DataRef indicatedAltitudeFt(dataRefs, "sim/flightmodel/misc/h_ind");
static DataRef actualAltitudeMeter(dataRefs, "sim/flightmodel/position/elevation");
static DataRef aglAltitudeMeter(dataRefs, "sim/flightmodel/position/y_agl");

static DataRef autopilotAltitudeFt(dataRefs, "sim/cockpit/autopilot/altitude");

// Heading
static DataRef headingTrueDeg(dataRefs, "sim/flightmodel/position/true_psi");
static DataRef headingMagDeg(dataRefs, "sim/flightmodel/position/mag_psi");

// Misc
static DataRef numberOfEngines(dataRefs, "sim/aircraft/engine/acf_num_engines");
static DataRef onGround(dataRefs, "sim/flightmodel/failures/onground_any");
static DataRef rainPercentage(dataRefs, "sim/weather/rain_percent");

// Size in local coordinates (meter)
// points to the right side of the aircraft
static DataRef aircraftSizeX(dataRefs, "sim/aircraft/view/acf_size_x");
// points to the tail of the aircraft
static DataRef aircraftSizeZ(dataRefs, "sim/aircraft/view/acf_size_z");

// The two X-Plane ships - Index 0=carrier,1=frigate ===========================
// Heading of the boat in degrees from true north
static DataRef boatHeadingDeg(dataRefs, "sim/world/boat/heading_deg");

// Deck height of the frigate (in coordinates of the OBJ model)
static DataRef boatFrigateDeckHeightMtr(dataRefs, "sim/world/boat/frigate_deck_height_mtr");

// Deck height of the carrier (in coordinates of the OBJ model)
static DataRef boatCarrierDeckHeightMtr(dataRefs, "sim/world/boat/carrier_deck_height_mtr");

// Velocity of the boat in meters per second in its current direction (value is always null in 11.41 and 11.50)
static DataRef boatVelocityMsc(dataRefs, "sim/world/boat/velocity_msc");

// Position of the boat in meters in the local coordinate OpenGL coordinate system.
static DataRef boatXMtr(dataRefs, "sim/world/boat/x_mtr");
static DataRef boatYMtr(dataRefs, "sim/world/boat/y_mtr");
static DataRef boatZMtr(dataRefs, "sim/world/boat/z_mtr");

static DataRef engineType8(dataRefs, "sim/aircraft/prop/acf_en_type");
// 0 = recip carb, 1 = recip injected, 2 = free turbine, 3 = electric, 4 = lo bypass jet, 5 = hi bypass jet, 6 = rocket, 7 = tip rockets, 8 = fixed turbine
enum XpEngineType
{
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

static QVector<MultiplayerDataRefs> multiplayerDataRefs;

// AI values - will be updated with number
static const QLatin1Literal HEADING_DEG_TRUE_AI("sim/multiplayer/position/plane%1_psi");
static const QLatin1Literal LAT_POSITION_DEG_AI("sim/multiplayer/position/plane%1_lat");
static const QLatin1Literal LON_POSITION_DEG_AI("sim/multiplayer/position/plane%1_lon");
static const QLatin1Literal ACTUAL_ALTITUDE_METER_AI("sim/multiplayer/position/plane%1_el");

// ============================================================
// New TCAS AI/multiplayer interface
namespace tcas {
// int integer If TCAS is not overriden by plgugin, returns the number of planes in X-Plane, which might be under plugin control or X-Plane control. If TCAS is overriden, returns how many targets are actually being written to with the override. These are not necessarily consecutive entries in the TCAS arrays.
static DataRef tcasNumAcf(dataRefs, "sim/cockpit2/tcas/indicators/tcas_num_acf");

// int[64] integer 24bit (0-16777215 or 0 - 0xFFFFFF) unique ID of the airframe. This is also known as the ADS-B "hexcode".
// static DataRef modeSId(dataRefs, "sim/cockpit2/tcas/targets/modeS_id");

// int[64] integer Mode C transponder code 0000 to 7777. This is not really an integer, this is an octal number.
static DataRef modeC_code(dataRefs, "sim/cockpit2/tcas/targets/modeC_code");

// float[64] degrees global coordinate, degrees.
static DataRef lat(dataRefs, "sim/cockpit2/tcas/targets/position/lat");

// float[64] degrees global coordinate, degrees.
static DataRef lon(dataRefs, "sim/cockpit2/tcas/targets/position/lon");

// float[64] meter global coordinate, meter.
static DataRef ele(dataRefs, "sim/cockpit2/tcas/targets/position/ele");

// float[64] feet/min absolute vertical speed feet per minute.
static DataRef verticalSpeed(dataRefs, "sim/cockpit2/tcas/targets/position/vertical_speed");

// float[64] meter/s total true speed, norm of local velocity vector. That means it includes vertical speed
static DataRef vMsc(dataRefs, "sim/cockpit2/tcas/targets/position/V_msc");

// float[64] degrees true heading orientation.
static DataRef psi(dataRefs, "sim/cockpit2/tcas/targets/position/psi");
}
}

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
}

bool XpConnect::fillSimConnectData(atools::fs::sc::SimConnectData& data, bool fetchAi)
{
  atools::fs::sc::SimConnectUserAircraft& userAircraft = data.userAircraft;

  float actualAlt = meterToFeet(dr::actualAltitudeMeter.valueFloat());
  userAircraft.position =
    Pos(dr::lonPositionDeg.valueFloat(), dr::latPositionDeg.valueFloat(), actualAlt);

  if(!userAircraft.position.isValid() || userAircraft.position.isNull())
    return false;

  userAircraft.magVarDeg = -dr::magVarDeg.valueFloat();

  userAircraft.numberOfEngines = static_cast<quint8>(dr::numberOfEngines.valueInt());

  // Wind and ambient parameters
  userAircraft.windSpeedKts = dr::windSpeedKts.valueFloat();
  userAircraft.windDirectionDegT = dr::windDirectionDegMag.valueFloat() + userAircraft.magVarDeg;
  userAircraft.ambientTemperatureCelsius = dr::ambientTemperatureC.valueFloat();
  userAircraft.totalAirTemperatureCelsius = dr::leTemperatureC.valueFloat();
  userAircraft.seaLevelPressureMbar = dr::seaLevelPressurePascal.valueFloat() / 100.f;

  // Ice
  userAircraft.pitotIcePercent = static_cast<quint8>(dr::pitotIcePercent.valueFloat() * 100.f);
  userAircraft.structuralIcePercent = static_cast<quint8>(std::max(dr::structuralIcePercent.valueFloat(),
                                                                   dr::structuralIcePercent2.valueFloat()) * 100.f);
  userAircraft.aoaIcePercent = static_cast<quint8>(std::max(dr::aoaIcePercent.valueFloat(),
                                                            dr::aoaIcePercent2.valueFloat()) * 100.f);
  userAircraft.inletIcePercent = static_cast<quint8>(dr::inletIcePercent.valueFloat() * 100.f);
  userAircraft.propIcePercent = static_cast<quint8>(dr::propIcePercent.valueFloat() * 100.f);
  userAircraft.statIcePercent = static_cast<quint8>(std::max(dr::statIcePercent.valueFloat(),
                                                             dr::statIcePercent2.valueFloat()) * 100.f);
  userAircraft.windowIcePercent = static_cast<quint8>(dr::windowIcePercent.valueFloat() * 100.f);

  userAircraft.carbIcePercent = 0.f;
  FloatVector carbIce = dr::carbIcePercent.valueFloatArr();
  for(int i = 0; i < carbIce.size() && i < userAircraft.numberOfEngines; i++)
    userAircraft.carbIcePercent = static_cast<quint8>(std::max(carbIce.value(i, 0.f) * 100.f,
                                                               static_cast<float>(userAircraft.carbIcePercent)));

  // Weight
  userAircraft.airplaneTotalWeightLbs = kgToLbs(dr::airplaneTotalWeightKgs.valueFloat());
  userAircraft.airplaneMaxGrossWeightLbs = kgToLbs(dr::airplaneMaxGrossWeightKgs.valueFloat());
  userAircraft.airplaneEmptyWeightLbs = kgToLbs(dr::airplaneEmptyWeightKgs.valueFloat());

  // Fuel flow in weight
  userAircraft.fuelTotalWeightLbs = kgToLbs(dr::fuelTotalWeightKgs.valueFloat());
  userAircraft.fuelFlowPPH = kgToLbs(dr::fuelFlowKgSec8.valueFloatArrSum()) * 3600.f;

  userAircraft.ambientVisibilityMeter = dr::ambientVisibilityM.valueFloat();

  // Build local time and use timezone offset from simulator
  // X-Plane does not allow to set the year
  userAircraft.localDateTime = atools::correctDateLocal(dr::localDateDays.valueInt(),
                                                        atools::roundToInt(dr::localTimeSec.valueFloat()),
                                                        atools::roundToInt(dr::zuluTimeSec.valueFloat()));
  userAircraft.zuluDateTime = userAircraft.localDateTime.toUTC();

  // SimConnectAircraft
  userAircraft.airplaneTitle = dr::airplaneTitle.valueString();
  userAircraft.airplaneModel = dr::airplaneType.valueString();
  userAircraft.airplaneReg = dr::airplaneTailnum.valueString();
  // userAircraft.airplaneType;           // not available - use model ICAO code in client
  // not available:
  // userAircraft.airplaneAirline; userAircraft.airplaneFlightnumber; userAircraft.fromIdent; userAircraft.toIdent;

  userAircraft.altitudeAboveGroundFt = meterToFeet(dr::aglAltitudeMeter.valueFloat());
  userAircraft.groundAltitudeFt = actualAlt - userAircraft.altitudeAboveGroundFt;
  userAircraft.altitudeAutopilotFt = dr::autopilotAltitudeFt.valueFloat();
  userAircraft.indicatedAltitudeFt = dr::indicatedAltitudeFt.valueFloat();

  // Heading and track
  userAircraft.headingMagDeg = dr::headingMagDeg.valueFloat();
  userAircraft.headingTrueDeg = dr::headingTrueDeg.valueFloat();
  userAircraft.trackMagDeg = dr::trackMagDeg.valueFloat();
  userAircraft.trackTrueDeg = userAircraft.trackMagDeg + userAircraft.magVarDeg;

  // Speed
  userAircraft.indicatedSpeedKts = dr::indicatedSpeedKts.valueFloat();
  userAircraft.trueAirspeedKts = meterToNm(dr::trueSpeedMs.valueFloat() * 3600.f);
  userAircraft.machSpeed = dr::machSpeed.valueFloat();
  userAircraft.verticalSpeedFeetPerMin = dr::verticalSpeedFpm.valueFloat();
  userAircraft.groundSpeedKts = meterToNm(dr::groundSpeedMs.valueFloat() * 3600.f);

  // Get transponder code and Convert decimals to octal code
  userAircraft.transponderCode = atools::fs::util::decodeTransponderCode(dr::transponderCode.valueInt());

  // Model
  // points to the tail of the aircraft
  userAircraft.modelRadiusFt = static_cast<quint16>(roundToInt(meterToFeet(dr::aircraftSizeZ.valueFloat())));

  // points to the right side of the aircraft - wingspan will be used before model radius for painting
  userAircraft.wingSpanFt = static_cast<quint16>(roundToInt(meterToFeet(dr::aircraftSizeX.valueFloat() * 2.)));

  // Set misc flags
  userAircraft.flags = atools::fs::sc::IS_USER | atools::fs::sc::SIM_XPLANE;
  if(dr::onGround.valueInt() > 0)
    userAircraft.flags |= atools::fs::sc::ON_GROUND;

  if(dr::rainPercentage.valueFloat() > 0.1f)
    userAircraft.flags |= atools::fs::sc::IN_RAIN;
  // IN_CLOUD = 0x0002, - not available
  // IN_SNOW = 0x0008,  - not available

  if(dr::simPaused.valueInt() > 0)
    userAircraft.flags |= atools::fs::sc::SIM_PAUSED;

  if(dr::simReplay.valueInt() > 0)
    userAircraft.flags |= atools::fs::sc::SIM_REPLAY;

  // Category is not available - Disable display of category on client
  userAircraft.category = atools::fs::sc::UNKNOWN;
  // AIRPLANE, HELICOPTER, BOAT, GROUNDVEHICLE, CONTROLTOWER, SIMPLEOBJECT, VIEWER

  // Value to calculate fuel volume from mass
  float fuelMassToVolDivider = 6.f;

  // Get the engine array
  IntVector engines = dr::engineType8.valueIntArr();
  userAircraft.engineType = atools::fs::sc::UNSUPPORTED;
  // PISTON = 0, JET = 1, NO_ENGINE = 2, HELO_TURBINE = 3, UNSUPPORTED = 4, TURBOPROP = 5

  // Get engine type
  for(int i = 0; i < engines.size() && i < userAircraft.numberOfEngines; i++)
  {
    dr::XpEngineType type = static_cast<dr::XpEngineType>(engines.value(i, xpc::dr::RECIP_CARB));
    switch(type)
    {
      case xpc::dr::ELECTRIC:
      case xpc::dr::RECIP_CARB:
      case xpc::dr::RECIP_INJECTED:
        userAircraft.engineType = atools::fs::sc::PISTON;
        fuelMassToVolDivider = 6.f; // Avgas lbs to gallons at standard temp
        break;

      case xpc::dr::FREE_TURBINE:
      case xpc::dr::FIXED_TURBINE:
      case xpc::dr::NEW_FREE_TURBINE:
      case xpc::dr::NEW_FIXED_TURBINE:
        fuelMassToVolDivider = 6.7f; // JetA lbs to gallons at standard temp
        userAircraft.engineType = atools::fs::sc::TURBOPROP;
        break;

      case xpc::dr::ROCKET:
      case xpc::dr::TIP_ROCKETS:
      case xpc::dr::LO_BYPASS_JET:
      case xpc::dr::HI_BYPASS_JET:
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
    FloatVector headings = dr::boatHeadingDeg.valueFloatArr();
    FloatVector velocity = dr::boatVelocityMsc.valueFloatArr();
    FloatVector x = dr::boatXMtr.valueFloatArr();
    FloatVector y = dr::boatYMtr.valueFloatArr();
    FloatVector z = dr::boatZMtr.valueFloatArr();

    // Add aircraft carrier =============================================================
    if(headings.size() > 0 && velocity.size() > 0 && x.size() > 0 && y.size() > 0 && z.size() > 0)
    {
      const static int CARRIER_IDX = 0;
      atools::fs::sc::SimConnectAircraft carrier;
      carrier.deckHeight = static_cast<quint16>(atools::geo::meterToFeet(dr::boatCarrierDeckHeightMtr.valueFloat()));
      carrier.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
      carrier.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

      carrier.flags = atools::fs::sc::SIM_XPLANE;

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
      frigate.deckHeight = static_cast<quint16>(atools::geo::meterToFeet(dr::boatFrigateDeckHeightMtr.valueFloat()));
      frigate.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
      frigate.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

      frigate.flags = atools::fs::sc::SIM_XPLANE;
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
    int numTcasAircraft = dr::tcas::tcasNumAcf.valueInt();
    bool hasTcasScheme = dr::tcas::modeC_code.isValid();

    // Get AI or multiplayer aircraft ===============================
    if(hasTcasScheme && numTcasAircraft > 0)
    {
      // Use new TCAS scheme - index 0 is user ======================
      for(int i = 1; i < numTcasAircraft; i++)
      {
        Pos pos(dr::tcas::lon.valueFloatArr(i), dr::tcas::lat.valueFloatArr(i),
                meterToFeet(dr::tcas::ele.valueFloatArr(i)));
        if(pos.isValid() && !pos.isNull())
        {
          // Coordinates are ok too - must be an AI aircraft
          atools::fs::sc::SimConnectAircraft aircraft;
          aircraft.flags = atools::fs::sc::SIM_XPLANE;
          aircraft.position = pos;
          aircraft.headingTrueDeg = dr::tcas::psi.valueFloatArr(i);

          // Mark fields as unavailable
          aircraft.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
          aircraft.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;

          // Ignore the vertical component
          aircraft.trueAirspeedKts = atools::geo::meterPerSecToKnots(dr::tcas::vMsc.valueFloatArr(i));

          aircraft.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;

          aircraft.verticalSpeedFeetPerMin = dr::tcas::verticalSpeed.valueFloatArr(i);

          // Get transponder code and Convert decimals to octal code
          aircraft.transponderCode = atools::fs::util::decodeTransponderCode(dr::tcas::modeC_code.valueIntArr(i));

          aircraft.objectId = objId;
          // aircraft.objectId = static_cast<quint32>(dr::tcas::modeSId.valueIntArr(i));

          aircraft.category = atools::fs::sc::AIRPLANE;
          aircraft.engineType = atools::fs::sc::UNSUPPORTED;

          fileLoader->loadAircraftFile(aircraft, static_cast<quint32>(i + 1));

          data.aiAircraft.append(aircraft);

          objId++;
        }
      }
    }
    else
    {
      // Use old multplayer scheme ======================

      // Includes user aircraft - can return more than 20 despite providing only datarefs 1-19 (minus user)
      int numAi = std::min(getNumActiveAircraft(), 20) - 1;

      for(int i = 0; i < numAi; i++)
      {
        const dr::MultiplayerDataRefs& ref = dr::multiplayerDataRefs.at(i);

        Pos pos(ref.lonPositionDegAi.valueFloat(), ref.latPositionDegAi.valueFloat(),
                meterToFeet(ref.actualAltitudeMeterAi.valueFloat()));

        if(pos.isValid() && !pos.isNull())
        {
          // Coordinates are ok too - must be an AI aircraft
          atools::fs::sc::SimConnectAircraft aircraft;
          aircraft.flags = atools::fs::sc::SIM_XPLANE;
          aircraft.position = pos;
          aircraft.headingTrueDeg = ref.headingTrueDegAi.valueFloat();

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

          fileLoader->loadAircraftFile(aircraft, static_cast<quint32>(i + 1));

          data.aiAircraft.append(aircraft);

          objId++;
        }
      } // for(int i = 0; i < numAi; i++)
    } // if(numTcasAircraft > 0) ... else
  } // if(fetchAi)

  return true;
}

void XpConnect::initDataRefs()
{
  // Initialize datarefs for the 19 AI aircraft first
  for(int i = 1; i < 20; i++)
  {
    dr::MultiplayerDataRefs refs;
    refs.headingTrueDegAi.setName(QString(dr::HEADING_DEG_TRUE_AI).arg(i));
    refs.latPositionDegAi.setName(QString(dr::LAT_POSITION_DEG_AI).arg(i));
    refs.lonPositionDegAi.setName(QString(dr::LON_POSITION_DEG_AI).arg(i));
    refs.actualAltitudeMeterAi.setName(QString(dr::ACTUAL_ALTITUDE_METER_AI).arg(i));

    // Add to the list
    dr::multiplayerDataRefs.append(refs);

    // Find them all
    dr::MultiplayerDataRefs& r = dr::multiplayerDataRefs.last();
    r.headingTrueDegAi.find();
    r.latPositionDegAi.find();
    r.lonPositionDegAi.find();
    r.actualAltitudeMeterAi.find();
  }

  // Find remaining datarefs of user aircraft
  for(DataRef *ref : dr::dataRefs)
  {
    // qDebug() << ref->getName();

    if(!ref->isValid())
      ref->find();
  }
}

} // namespace xpc
