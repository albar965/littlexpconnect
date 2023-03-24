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

#ifndef XPMENU_H
#define XPMENU_H

#include <QByteArray>

struct XpMenusPrivate;
class QString;

/*
 * Creates Xpconnect menu structure in XP, handles menu callback and updates values.
 */
class XpMenu
{
public:
  XpMenu();
  ~XpMenu();

  /* Add menu with fetch AI and fetch rate items. Menu items are checked according to loaded settings */
  void addMenu(const QString& menuName);

  XpMenu(const XpMenu& other) = delete;
  XpMenu& operator=(const XpMenu& other) = delete;

  /* Fetch rate as set in menus */
  int getFetchRateMs() const
  {
    return fetchRateMs;
  }

  /* Fetch AI if set in menus */
  bool isFetchAi() const
  {
    return fetchAi;
  }

  /* Load acf files */
  bool isFetchAircraftInfo() const
  {
    return fetchAiAircraftInfo;
  }

  /* Restore values from settings. Call before addMenu() */
  void restoreState();

  /* Save to settings */
  void saveState();

private:
  void menuHandler(int menuId, int itemIndex);

  /* Calls method above itemRefParam contains instance */
  static void menuHandlerInternal(void *, void *itemRefParam);

  int fetchRateMs = 200;
  bool fetchAi = true, fetchAiAircraftInfo = true;

  XpMenusPrivate *p;
  QByteArray menuName, versionName;
};

#endif // XPMENU_H
