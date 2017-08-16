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

#include "dataref.h"

#include "atools.h"

#include <QDebug>

DataRef::DataRef(DataRefPtrVector& refs, const QString& dataRefName)
  : name(dataRefName)
{
  refs.append(this);
}

DataRef::DataRef(const QString& dataRefName)
  : name(dataRefName)
{
}

DataRef::DataRef()
{

}

bool DataRef::resolve()
{
  dataRef = XPLMFindDataRef(name.toLatin1());

  if(dataRef == NULL)
  {
    qWarning() << "Cannot find dataref" << name;
    return false;
  }

  dataRefType = XPLMGetDataRefTypes(dataRef);
  return true;
}

bool DataRef::isValid() const
{
  return !name.isEmpty() && dataRef != NULL && dataRefType != xplmType_Unknown;
}

void DataRef::clear()
{
  name.clear();
  dataRef = NULL;
  dataRefType = xplmType_Unknown;
}

QVector<int> DataRef::valueIntArr() const
{
  int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
  IntVector retval(size);
  XPLMGetDatavi(dataRef, retval.data(), 0, size);
  return retval;
}

QVector<float> DataRef::valueFloatArr() const
{
  int size = XPLMGetDatavf(dataRef, nullptr, 0, 0);
  FloatVector retval(size);
  XPLMGetDatavf(dataRef, retval.data(), 0, size);
  return retval;
}

QByteArray DataRef::valueByteArr() const
{
  int size = XPLMGetDatab(dataRef, nullptr, 0, 0);
  QByteArray retval(size, 0);
  XPLMGetDatab(dataRef, retval.data(), 0, size);
  return retval;
}

void DataRef::valueIntArr(IntVector& array) const
{
  int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
  array.resize(size);
  XPLMGetDatavi(dataRef, array.data(), 0, size);
}

void DataRef::valueFloatArr(FloatVector& array) const
{
  int size = XPLMGetDatavf(dataRef, nullptr, 0, 0);
  array.resize(size);
  XPLMGetDatavf(dataRef, array.data(), 0, size);
}

void DataRef::valueByteArr(QByteArray& bytes) const
{
  int size = XPLMGetDatab(dataRef, nullptr, 0, 0);
  bytes.resize(size);
  XPLMGetDatab(dataRef, bytes.data(), 0, size);
}

QString DataRef::valueString() const
{
  return QString(valueByteArr());
}

int DataRef::valueIntArrSum() const
{
  int sumValue = 0.f;
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

int DataRef::valueIntArrAvg() const
{
  int sumValue = 0.f;
  IntVector arr = valueIntArr();
  for(float val : arr)
    sumValue += val;
  return atools::roundToInt(static_cast<float>(sumValue) / static_cast<float>(arr.size()));
}

float DataRef::valueFloatArrAvg() const
{
  float sumValue = 0.f;
  FloatVector arr = valueFloatArr();
  for(float val : arr)
    sumValue += val;
  return sumValue / arr.size();
}
