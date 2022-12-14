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

#include "xpdatarefs.h"

namespace xpc {

void XpDataRefs::init()
{
  // AI values - will be updated with number
  const static QLatin1String HEADING_DEG_TRUE_AI("sim/multiplayer/position/plane%1_psi");
  const static QLatin1String LAT_POSITION_DEG_AI("sim/multiplayer/position/plane%1_lat");
  const static QLatin1String LON_POSITION_DEG_AI("sim/multiplayer/position/plane%1_lon");
  const static QLatin1String ACTUAL_ALTITUDE_METER_AI("sim/multiplayer/position/plane%1_el");

  // This is the internal build number - it is a unique integer that always increases and is unique with each beta.
  // For example, 10.51b5 might be 105105.
  // There is no guarantee that the build number (last 2 digits) is in sync with the official beta number.
  xplmVersion.init("sim/version/xplane_internal_version");
  xplmVersion.find();

  // Dataref 'sim/weather/temperature_ambient_c' has been replaced. Please use the new name.
  // Dataref 'sim/weather/temperature_le_c' has been replaced. Please use the new name.
  // Dataref 'sim/weather/visibility_reported_m' has been replaced. Please use the new name.
  // Dataref 'sim/weather/rain_percent' has been replaced. Please use the new name.
  // New "sim/weather/aircraft"
  bool xp12 = isXplane12();

  simPaused.init(dataRefs, "sim/time/paused");
  simReplay.init(dataRefs, "sim/operation/prefs/replay_mode");

  // SimConnectUserAircraft
  windSpeedKts.init(dataRefs, "sim/cockpit2/gauges/indicators/wind_speed_kts");
  windDirectionDegMag.init(dataRefs, "sim/cockpit2/gauges/indicators/wind_heading_deg_mag");

  // Temperatures
  // air temperature (SAT) is also called: outside air temperature (OAT) or true air temperature
  // Lower than TAT
  ambientTemperatureC.init(dataRefs, xp12 ? "sim/weather/aircraft/temperature_ambient_deg_c" : "sim/weather/temperature_ambient_c");

  // Total air temperature (TAT) is also called: indicated air temperature (IAT) or ram air temperature (RAT)
  // higher than SAT
  leTemperatureC.init(dataRefs, xp12 ? "sim/weather/aircraft/temperature_leadingedge_deg_c" : "sim/weather/temperature_le_c");

  seaLevelPressurePascal.init(dataRefs, "sim/physics/earth_pressure_p");

  // Ice
  pitotIcePercent.init(dataRefs, "sim/flightmodel/failures/pitot_ice");
  structuralIcePercent.init(dataRefs, "sim/flightmodel/failures/frm_ice");
  structuralIcePercent2.init(dataRefs, "sim/flightmodel/failures/frm_ice2");
  aoaIcePercent.init(dataRefs, "sim/flightmodel/failures/aoa_ice");
  aoaIcePercent2.init(dataRefs, "sim/flightmodel/failures/aoa_ice2");
  inletIcePercent.init(dataRefs, "sim/flightmodel/failures/inlet_ice");
  propIcePercent.init(dataRefs, "sim/flightmodel/failures/prop_ice");
  statIcePercent.init(dataRefs, "sim/flightmodel/failures/stat_ice");
  statIcePercent2.init(dataRefs, "sim/flightmodel/failures/stat_ice2");
  windowIcePercent.init(dataRefs, "sim/flightmodel/failures/window_ice");
  carbIcePercent.init(dataRefs, "sim/flightmodel/engine/ENGN_crbice");

  // Weight
  airplaneTotalWeightKgs.init(dataRefs, "sim/flightmodel/weight/m_total");
  airplaneMaxGrossWeightKgs.init(dataRefs, "sim/aircraft/weight/acf_m_max");
  airplaneEmptyWeightKgs.init(dataRefs, "sim/aircraft/weight/acf_m_empty");
  airplanePayloadWeightKgs.init(dataRefs, "sim/flightmodel/weight/m_fixed");

  // fuelTotalQuantityGallons.init(dataRefs, ""); value calculated
  fuelTotalWeightKgs.init(dataRefs, "sim/flightmodel/weight/m_fuel_total");

  // Value for up to eight engines
  fuelFlowKgSec8.init(dataRefs, "sim/cockpit2/engine/indicators/fuel_flow_kg_sec");
  // fuelFlowGPH.init(dataRefs, ""); value calculated

  magVarDeg.init(dataRefs, "sim/flightmodel/position/magnetic_variation");
  ambientVisibility.init(dataRefs, xp12 ? "sim/weather/aircraft/visibility_reported_sm" : "sim/weather/visibility_reported_m");
  trackMagDeg.init(dataRefs, "sim/cockpit2/gauges/indicators/ground_track_mag_pilot");
  // trackTrueDeg.init(dataRefs, ""); value calculated

  // Date and time
  localDateDays.init(dataRefs, "sim/time/local_date_days");
  localTimeSec.init(dataRefs, "sim/time/local_time_sec");
  zuluTimeSec.init(dataRefs, "sim/time/zulu_time_sec");

  // SimConnectAircraft
  airplaneTailnum.init(dataRefs, "sim/aircraft/view/acf_tailnum");
  airplaneTitle.init(dataRefs, "sim/aircraft/view/acf_descrip");
  airplaneType.init(dataRefs, "sim/aircraft/view/acf_ICAO");
  transponderCode.init(dataRefs, "sim/cockpit/radios/transponder_code");

  // Position
  latPositionDeg.init(dataRefs, "sim/flightmodel/position/latitude");
  lonPositionDeg.init(dataRefs, "sim/flightmodel/position/longitude");

  // Speeds
  indicatedSpeedKts.init(dataRefs, "sim/flightmodel/position/indicated_airspeed");
  trueSpeedMs.init(dataRefs, "sim/flightmodel/position/true_airspeed");
  groundSpeedMs.init(dataRefs, "sim/flightmodel/position/groundspeed");
  machSpeed.init(dataRefs, "sim/flightmodel/misc/machno");
  verticalSpeedFpm.init(dataRefs, "sim/flightmodel/position/vh_ind_fpm");

  // Altitude
  indicatedAltitudeFt.init(dataRefs, "sim/flightmodel/misc/h_ind");
  actualAltitudeMeter.init(dataRefs, "sim/flightmodel/position/elevation");
  aglAltitudeMeter.init(dataRefs, "sim/flightmodel/position/y_agl");

  autopilotAltitudeFt.init(dataRefs, "sim/cockpit/autopilot/altitude");

  // Heading
  headingTrueDeg.init(dataRefs, "sim/flightmodel/position/true_psi");
  headingMagDeg.init(dataRefs, "sim/flightmodel/position/mag_psi");

  // Misc
  numberOfEngines.init(dataRefs, "sim/aircraft/engine/acf_num_engines");
  onGround.init(dataRefs, "sim/flightmodel/failures/onground_any");
  rainPercentage.init(dataRefs, xp12 ? "sim/weather/aircraft/precipitation_on_aircraft_ratio" : "sim/weather/rain_percent");

  // Size in local coordinates (meter)
  // points to the right side of the aircraft
  aircraftSizeX.init(dataRefs, "sim/aircraft/view/acf_size_x");
  // points to the tail of the aircraft
  aircraftSizeZ.init(dataRefs, "sim/aircraft/view/acf_size_z");

  // The two X-Plane ships - Index 0=carrier,1=frigate ===========================
  // Heading of the boat in degrees from true north
  boatHeadingDeg.init(dataRefs, "sim/world/boat/heading_deg");

  // Deck height of the frigate (in coordinates of the OBJ model)
  boatFrigateDeckHeightMtr.init(dataRefs, "sim/world/boat/frigate_deck_height_mtr");

  // Deck height of the carrier (in coordinates of the OBJ model)
  boatCarrierDeckHeightMtr.init(dataRefs, "sim/world/boat/carrier_deck_height_mtr");

  // Velocity of the boat in meters per second in its current direction (value is always null in 11.41 and 11.50)
  boatVelocityMsc.init(dataRefs, "sim/world/boat/velocity_msc");

  // Position of the boat in meters in the local coordinate OpenGL coordinate system.
  boatXMtr.init(dataRefs, "sim/world/boat/x_mtr");
  boatYMtr.init(dataRefs, "sim/world/boat/y_mtr");
  boatZMtr.init(dataRefs, "sim/world/boat/z_mtr");

  engineType8.init(dataRefs, "sim/aircraft/prop/acf_en_type");

  // ============================================================
  // New TCAS AI/multiplayer interface
  // int integer If TCAS is not overriden by plgugin, returns the number of planes in X-Plane, which might be under plugin control or X-Plane control. If TCAS is overriden, returns how many targets are actually being written to with the override. These are not necessarily consecutive entries in the TCAS arrays.
  tcasNumAcf.init(dataRefs, "sim/cockpit2/tcas/indicators/tcas_num_acf");

  // int[64] integer 24bit (0-16777215 or 0 - 0xFFFFFF) unique ID of the airframe. This is also known as the ADS-B "hexcode".
  tcasModeSId.init(dataRefs, "sim/cockpit2/tcas/targets/modeS_id");

  // int[64] integer Mode C transponder code 0000 to 7777. This is not really an integer, this is an octal number.
  tcasModeCcode.init(dataRefs, "sim/cockpit2/tcas/targets/modeC_code");

  // float[64] degrees global coordinate, degrees.
  tcasLat.init(dataRefs, "sim/cockpit2/tcas/targets/position/lat");

  // float[64] degrees global coordinate, degrees.
  tcasLon.init(dataRefs, "sim/cockpit2/tcas/targets/position/lon");

  // float[64] meter global coordinate, meter.
  tcasEle.init(dataRefs, "sim/cockpit2/tcas/targets/position/ele");

  // float[64] feet/min absolute vertical speed feet per minute.
  tcasVerticalSpeed.init(dataRefs, "sim/cockpit2/tcas/targets/position/vertical_speed");

  // float[64] meter/s total true speed, norm of local velocity vector. That means it includes vertical speed
  tcasVMsc.init(dataRefs, "sim/cockpit2/tcas/targets/position/V_msc");

  // float[64] degrees true heading orientation.
  tcasPsi.init(dataRefs, "sim/cockpit2/tcas/targets/position/psi");

  // int[64]	boolean	ground/flight logic. Writeable only when override_TCAS is set.
  tcasWeightOnWheels.init(dataRefs, "sim/cockpit2/tcas/targets/position/weight_on_wheels");

  // sim/cockpit2/tcas/targets/icao_type	byte[512]	y	string	7 character ICAO code, terminated by 0 byte. C172, B738, etc...
  // see https://www.icao.int/publications/DOC8643/Pages/Search.aspx
  tcasIcaoType.init(dataRefs, "sim/cockpit2/tcas/targets/icao_type");

  // sim/cockpit2/tcas/targets/wake/wing_span_m	float[64]	y	meter	wing span of the aircraft creating wake turbulence

  // ============================================================
  // Old AI/multiplayer interface
  // Initialize datarefs for the 19 AI aircraft first
  for(int i = 1; i < 20; i++)
  {
    MultiplayerDataRefs refs;
    refs.headingTrueDegAi.setName(QString(HEADING_DEG_TRUE_AI).arg(i));
    refs.latPositionDegAi.setName(QString(LAT_POSITION_DEG_AI).arg(i));
    refs.lonPositionDegAi.setName(QString(LON_POSITION_DEG_AI).arg(i));
    refs.actualAltitudeMeterAi.setName(QString(ACTUAL_ALTITUDE_METER_AI).arg(i));

    // Add to the list
    multiplayerDataRefs.append(refs);

    // Find them all
    MultiplayerDataRefs& r = multiplayerDataRefs.last();
    r.headingTrueDegAi.find();
    r.latPositionDegAi.find();
    r.lonPositionDegAi.find();
    r.actualAltitudeMeterAi.find();
  }

  // Find remaining datarefs of user aircraft
  for(DataRef *ref : dataRefs)
  {
    if(!ref->isValid())
      ref->find();
  }
}

} // namespace xpc
