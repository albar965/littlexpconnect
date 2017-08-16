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

#ifndef LITTLEXPC_DATAREF_H
#define LITTLEXPC_DATAREF_H

#include <QString>
#include <QVariant>

extern "C" {
#include "XPLMDataAccess.h"
}

class DataRef;

typedef QVector<float> FloatVector;
typedef QVector<int> IntVector;
typedef QVector<DataRef *> DataRefPtrVector;

class DataRef
{
public:
  DataRef(DataRefPtrVector& refs, const QString& dataRefName);
  DataRef(const QString& dataRefName);
  DataRef();

  bool setRef(const QString& dataRefName);
  bool resolve();

  bool isValid() const;
  void clear();

  float valueFloat() const
  {
    return XPLMGetDataf(dataRef);
  }

  double valueDouble() const
  {
    return XPLMGetDatad(dataRef);
  }

  int valueInt() const
  {
    return XPLMGetDatai(dataRef);
  }

  QString valueString() const;

  int valueIntArrSum() const;
  float valueFloatArrSum() const;
  int valueIntArrAvg() const;
  float valueFloatArrAvg() const;

  IntVector valueIntArr() const;
  FloatVector valueFloatArr() const;
  QByteArray valueByteArr() const;

  void valueIntArr(IntVector& array) const;
  void valueFloatArr(FloatVector& array) const;
  void valueByteArr(QByteArray& bytes) const;

  XPLMDataTypeID getDataRefType() const
  {
    return dataRefType;
  }

  const QString& getName() const
  {
    return name;
  }

private:
  XPLMDataRef dataRef = NULL;
  XPLMDataTypeID dataRefType = 0;
  QString name;
};

#endif // LITTLEXPC_DATAREF_H
