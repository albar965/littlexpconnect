/*****************************************************************************
* Copyright 2015-2023 Alexander Barthel alex@littlenavmap.org
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

#include "xpconnect/dataref.h"

#include "geo/pos.h"

#include <QDebug>
#include <QDir>

extern "C" {
#include "XPLMPlanes.h"
#include "XPLMGraphics.h"
}

QString getAircraftModelFilepath(int index)
{
  char outFileName[2048]; // Filename only
  char outPath[2048]; // Full filepath including filename
  memset(outFileName, 0, 2048);
  memset(outPath, 0, 2048);

  XPLMGetNthAircraftModel(index, outFileName, outPath);

  return QDir::toNativeSeparators(QString(outPath));
}

atools::geo::Pos localToWorld(double x, double y, double z)
{
  double lat, lon, alt;
  XPLMLocalToWorld(x, y, z, &lat, &lon, &alt);
  return atools::geo::Pos(lon, lat, alt);
}

void worldToLocal(double& x, double& y, double& z, const atools::geo::Pos& pos)
{
  XPLMWorldToLocal(pos.getLatY(), pos.getLonX(), pos.getAltitude(), &x, &y, &z);
}

// ==========================================================================================
DataRef::DataRef(DataRefPtrVector& refs, const QString& dataRefName)
{
  init(refs, dataRefName);
}

DataRef::DataRef(const QString& dataRefName)
{
  init(dataRefName);
}

DataRef::DataRef()
{
}

void DataRef::init(DataRefPtrVector& refs, const QString& dataRefName)
{
  name = dataRefName;

  // Add to array but do not find it yet (still invalid)
  refs.append(this);
}

void DataRef::init(const QString& dataRefName)
{
  name = dataRefName;
}

bool DataRef::find(bool warnNotFound)
{
  dataRef = XPLMFindDataRef(name.toLatin1().constData());
  if(dataRef == nullptr)
  {
    if(warnNotFound)
      qWarning() << Q_FUNC_INFO << "Cannot find dataref" << name;
  }
  else
  {
    dataRefType = XPLMGetDataRefTypes(dataRef);
    if(dataRefType == xplmType_Unknown)
      qWarning() << Q_FUNC_INFO << name << "has unexpected type xplmType_Unknown";
  }

  return isValid();
}

QVector<int> DataRef::valueIntArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_IntArray);
#endif
  IntVector retval;

  if(dataRef != nullptr)
  {
    // Get size first by calling with null pointer
    int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
    retval.resize(size);

    // Get the array contents
    XPLMGetDatavi(dataRef, retval.data(), 0, size);
  }

  return retval;
}

QVector<float> DataRef::valueFloatArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_FloatArray);
#endif

  FloatVector retval;
  if(dataRef != nullptr)
  {
    // Get size first by calling with null pointer
    int size = XPLMGetDatavf(dataRef, nullptr, 0, 0);
    retval.resize(size);

    // Get the array contents
    XPLMGetDatavf(dataRef, retval.data(), 0, size);
  }

  return retval;
}

QByteArray DataRef::valueByteArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_Data);
#endif

  QByteArray retval;
  if(dataRef != nullptr)
  {
    // Get size first by calling with null pointer
    int size = XPLMGetDatab(dataRef, nullptr, 0, 0);
    retval.resize(size);

    // Get the array contents
    XPLMGetDatab(dataRef, retval.data(), 0, size);
  }
  return retval;
}

int DataRef::sizeIntArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_IntArray);
#endif

  if(dataRef != nullptr)
    return XPLMGetDatavi(dataRef, nullptr, 0, 0);
  else
    return 0;
}

int DataRef::sizeFloatArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_FloatArray);
#endif

  if(dataRef != nullptr)
    return XPLMGetDatavf(dataRef, nullptr, 0, 0);
  else
    return 0;
}

int DataRef::sizeByteArr() const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_Data);
#endif

  if(dataRef != nullptr)
    return XPLMGetDatab(dataRef, nullptr, 0, 0);
  else
    return 0;
}

int DataRef::valueIntArr(int index) const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_IntArray);
#endif

  int retval = 0;
  if(dataRef != nullptr)
    XPLMGetDatavi(dataRef, &retval, index, 1);
  return retval;
}

float DataRef::valueFloatArr(int index) const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_FloatArray);
#endif

  float retval = 0.f;
  if(dataRef != nullptr)
    XPLMGetDatavf(dataRef, &retval, index, 1);
  return retval;
}

int DataRef::valueByteArr(int index) const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_FloatArray);
#endif

  char retval = '\0';
  if(dataRef != nullptr)
    XPLMGetDatab(dataRef, &retval, index, 1);
  return retval;
}

void DataRef::valueIntArr(IntVector& array) const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_IntArray);
#endif

  if(dataRef != nullptr)
  {
    int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
    array.resize(size);
    XPLMGetDatavi(dataRef, array.data(), 0, size);
  }
  else
    array.clear();
}

void DataRef::valueFloatArr(FloatVector& array) const
{
#ifdef DATAREF_VALIDATION
  checkType(xplmType_FloatArray);
#endif

  if(dataRef != nullptr)
  {
    int size = XPLMGetDatavf(dataRef, nullptr, 0, 0);
    array.resize(size);
    XPLMGetDatavf(dataRef, array.data(), 0, size);
  }
  else
    array.clear();
}

void DataRef::valueByteArr(QByteArray& bytes) const
{
  if(dataRef != nullptr)
  {
    int size = XPLMGetDatab(dataRef, nullptr, 0, 0);
    bytes.resize(size);
    XPLMGetDatab(dataRef, bytes.data(), 0, size);
  }
  else
    bytes.clear();
}

#ifdef DATAREF_VALIDATION
void DataRef::checkType(int type) const
{
  if((dataRefType & type) == 0)
    qWarning() << Q_FUNC_INFO << "Type mismatch" << dataRefType << "does not cover" << type;
}

#endif

int DataRef::valueIntArrSum() const
{
  int sumValue = 0;
  for(int val : valueIntArr())
    sumValue += val;
  return sumValue;
}

float DataRef::valueFloatArrSum() const
{
  float sumValue = 0.f;
  for(float val : valueFloatArr())
    sumValue += val;

  return sumValue;
}

int getNumActiveAircraft()
{
  int outTotalAircraft, outActiveAircraft;
  XPLMPluginID outController;

  XPLMCountAircraft(&outTotalAircraft, &outActiveAircraft, &outController);
  return outActiveAircraft;
}
