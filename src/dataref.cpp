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

#include <QDebug>

DataRef::DataRef(const QString& dataRefName, QVariant::Type typeParam, int dataRefArraySize)
  : name(dataRefName), type(typeParam), arraySize(dataRefArraySize)
{
  dataRef = XPLMFindDataRef(dataRefName.toLatin1());
  dataRefType = XPLMGetDataRefTypes(dataRef);

  if(dataRef == NULL)
    qWarning() << "Cannot find dataref" << name;
}

QVector<int> DataRef::valueIntArr() const
{
  int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
  QVector<int> retval(size);
  XPLMGetDatavi(dataRef, retval.data(), 0, size);
  return retval;
}

QVector<float> DataRef::valueFloatArr() const
{
  int size = XPLMGetDatavf(dataRef, nullptr, 0, 0);
  QVector<float> retval(size);
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

void DataRef::valueIntArr(QVector<int>& array) const
{
  int size = XPLMGetDatavi(dataRef, nullptr, 0, 0);
  array.resize(size);
  XPLMGetDatavi(dataRef, array.data(), 0, size);
}

void DataRef::valueFloatArr(QVector<float>& array) const
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
