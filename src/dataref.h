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

#ifndef LITTLEXPC_DATAREF_H
#define LITTLEXPC_DATAREF_H

#include <QString>
#include <QVariant>

extern "C" {
#include "XPLMDataAccess.h"
}

namespace atools {
namespace geo {
class Pos;
}
}

class DataRef;

typedef QVector<float> FloatVector;
typedef QVector<int> IntVector;
typedef QVector<DataRef *> DataRefPtrVector;

/* Get full path to the acf file for the aircraft at the given index. 0 is the user aircraft. */
QString getAircraftModelFilepath(int index);

/* Read keys from acf file. Reading stops if all keys are found. Use rarely and cache values since
 * it can read up to 100000 lines of text. */
void readValuesFromAcfFile(QHash<QString, QString>& keyValuePairs, const QString& filepath, const QStringList& keys);

int getNumActiveAircraft();

/* The XYZ coordinates are in meters in the local OpenGL coordinate system.
 * Latitude and longitude are in decimal degrees and altitude is in meters MSL */
atools::geo::Pos localToWorld(double x, double y, double z);
void worldToLocal(double& x, double& y, double& z, const atools::geo::Pos& pos);

/*
 * Hides the XPLM data ref accessor methods and provides methods for easier access than the C interface.
 * The class allows only reading of datarefs.
 * The accessor methods can optionally check if the type is valid (define DATAREF_VALIDATION).
 */
class DataRef
{
public:
  /*
   * Initializes a dataref but does not call the find method yet.
   * DataRef is not valid yet after construction.
   *
   * @param refs Object adds itself to the list when constructed.
   * @param name Path/name of the dataref like "sim/aircraft/view/acf_tailnum".
   */
  DataRef(DataRefPtrVector& refs, const QString& dataRefName);
  DataRef(const QString& dataRefName);
  DataRef();

  void setName(const QString& dataRefName)
  {
    name = dataRefName;
  }

  /*
   * Calls the find method and return true if that dataref name was valid and found.
   * Prints a warning into the log if the ref could not be found.
   */
  bool find();

  /* returns true if ref was found. */
  bool isValid() const
  {
    return !name.isEmpty() && dataRef != nullptr && dataRefType != xplmType_Unknown;
  }

  /* get float value or 0 if invalid */
  float valueFloat() const
  {
#ifdef DATAREF_VALIDATION
    checkType(xplmType_Float);
#endif
    return XPLMGetDataf(dataRef);
  }

  /* get double value or 0 if invalid */
  double valueDouble() const
  {
#ifdef DATAREF_VALIDATION
    checkType(xplmType_Double);
#endif
    return XPLMGetDatad(dataRef);
  }

  /* get int value or 0 if invalid */
  int valueInt() const
  {
#ifdef DATAREF_VALIDATION
    checkType(xplmType_Int);
#endif
    return XPLMGetDatai(dataRef);
  }

  /* get string from an UTF-8 byte array value or empty string if invalid */
  QString valueString() const
  {
    return QString(valueByteArr());
  }

  /* Get the sum of all values in an array. */
  int valueIntArrSum() const;
  float valueFloatArrSum() const;

  /* Get arrays. The length of the array is checked before  retrieving the values.*/
  IntVector valueIntArr() const;
  FloatVector valueFloatArr() const;
  QByteArray valueByteArr() const;

  void valueIntArr(IntVector& array) const;
  void valueFloatArr(FloatVector& array) const;
  void valueByteArr(QByteArray& bytes) const;

  /* Get the type of the dataref after calling find */
  XPLMDataTypeID getDataRefType() const
  {
    return dataRefType;
  }

  /* Name of the dataref as passed to the constructor */
  const QString& getName() const
  {
    return name;
  }

private:
#ifdef DATAREF_VALIDATION
  void checkType(int type) const;

#endif

  XPLMDataRef dataRef = nullptr;
  XPLMDataTypeID dataRefType = 0;
  QString name;
};

#endif // LITTLEXPC_DATAREF_H
