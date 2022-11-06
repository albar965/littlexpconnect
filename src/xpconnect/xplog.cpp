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

#include "xpconnect/xplog.h"

extern "C" {
#include "XPLMUtilities.h"
}

#include <QCoreApplication>
#include <QStringBuilder>
#include <QDateTime>

namespace xplog {
void logXpInfo(QString message)
{
  message = "[" % QDateTime::currentDateTime().toString(Qt::ISODateWithMs) % " INFO] " %
            QCoreApplication::applicationName() % ": " % message % "\n";
  XPLMDebugString(message.toUtf8().constData());
}

void logXpErr(QString message)
{
  message = "[" % QDateTime::currentDateTime().toString(Qt::ISODateWithMs) % " ERROR] " %
            QCoreApplication::applicationName() % " error: " % message % "\n";
  XPLMDebugString(message.toUtf8().constData());
}

} // namespace xplog
