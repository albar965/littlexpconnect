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

#ifndef XPLOG_H
#define XPLOG_H

class QString;

/* Do not call this from threads */
namespace xplog {

/* Write log message with plugin name prefix into X-Plane "Log.txt" */
void logXpInfo(QString message);

/* Write error message with plugin name prefix into X-Plane "Log.txt" */
void logXpErr(QString message);

} // namespace xplog

#endif // XPLOG_H
