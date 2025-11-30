/*****************************************************************************
* Copyright 2015-2024 Alexander Barthel alex@littlenavmap.org
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

#include "xpmenu.h"

#include "settings/settings.h"

extern "C" {
#include "XPLMMenus.h"
}

#include <QCoreApplication>
#include <QDebug>

namespace lxc {
/* key names for atools::settings */
static const QLatin1String SETTINGS_OPTIONS_FETCH_RATE_MS("Options/FetchRateMs");
static const QLatin1String SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT("Options/FetchAiAircraft");
static const QLatin1String SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT_INFO("Options/FetchAiAircraftInfo");
}

enum MenuId
{
  VERSION = 0,
  FETCH_AI = 1,
  FETCH_AI_INFO = 2,
  FETCH_RATE_50 = 50,
  FETCH_RATE_100 = 100,
  FETCH_RATE_150 = 150,
  FETCH_RATE_200 = 200,
  FETCH_RATE_250 = 250,
  FETCH_RATE_500 = 500
};

const static int NUM_MENU_IDS = 9;

// Pointer to this is passed to the menu handler by a called menu item
struct Item
{
  MenuId menuId; // Own menu id enum
  int xpItemIndex; // X-Plane menu index as returned by XPLMAppendMenuItem
  XpMenu *instance; // Instance pointer
};

// Private structure to keep header clean
struct XpMenusPrivate
{
  XPLMMenuID xpMenuId; // The menu container we'll append all our menu items to
  Item items[NUM_MENU_IDS];
};

XpMenu::XpMenu()
{
  p = new XpMenusPrivate;
}

XpMenu::~XpMenu()
{
  XPLMDestroyMenu(p->xpMenuId);
  delete p;
}

void XpMenu::restoreState()
{
  atools::settings::Settings& settings = atools::settings::Settings::instance();
  fetchAi = settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool();
  fetchAiAircraftInfo = settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT_INFO, true).toBool();
  fetchRateMs = std::max(settings.getAndStoreValue(lxc::SETTINGS_OPTIONS_FETCH_RATE_MS, 200).toInt(), 50);
}

void XpMenu::saveState() const
{
  atools::settings::Settings& settings = atools::settings::Settings::instance();
  settings.setValue(lxc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, fetchAi);
  settings.setValue(lxc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT_INFO, fetchAiAircraftInfo);
  settings.setValue(lxc::SETTINGS_OPTIONS_FETCH_RATE_MS, fetchRateMs);
  atools::settings::Settings::syncSettings();
}

void XpMenu::menuHandlerInternal(void *, void *itemRefParam)
{
  const Item *itemRef = static_cast<Item *>(itemRefParam);
  itemRef->instance->menuHandler(itemRef->menuId, itemRef->xpItemIndex);
}

void XpMenu::menuHandler(int menuId, int itemIndex)
{
  if(menuId == FETCH_AI)
  {
    // Toggle AI menu item
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(p->xpMenuId, itemIndex, &check);
    XPLMCheckMenuItem(p->xpMenuId, itemIndex, check == xplm_Menu_Unchecked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    fetchAi = check == xplm_Menu_Unchecked;
  }
  else if(menuId == FETCH_AI_INFO)
  {
    // Toggle AI info menu item
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(p->xpMenuId, itemIndex, &check);
    XPLMCheckMenuItem(p->xpMenuId, itemIndex, check == xplm_Menu_Unchecked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    fetchAiAircraftInfo = check == xplm_Menu_Unchecked;
  }
  else if(menuId >= FETCH_RATE_50 && menuId <= FETCH_RATE_500)
  {
    // First deselect all rate items
    for(const Item& itemRef : p->items)
    {
      if(itemRef.menuId != FETCH_AI && itemRef.menuId != FETCH_AI_INFO)
        XPLMCheckMenuItem(p->xpMenuId, itemRef.xpItemIndex, xplm_Menu_Unchecked);
    }

    // Select rate item calling
    XPLMCheckMenuItem(p->xpMenuId, itemIndex, xplm_Menu_Checked);
    fetchRateMs = menuId;
  }
  qInfo() << "fetchAi" << fetchAi << "fetchAircraftInfo" << fetchAiAircraftInfo << "fetchRate" << fetchRateMs;
}

void XpMenu::addMenu(const QString& menuNameParam)
{
  versionName = QString("Version %1").arg(QCoreApplication::applicationVersion()).toLatin1();
  menuName = menuNameParam.toLatin1();

  // The index of our menu item in the Plugins menu
  int xpMenuContainterIndex = XPLMAppendMenuItem(XPLMFindPluginsMenu(), menuName.constData(), nullptr, 0);

  p->xpMenuId = XPLMCreateMenu(menuName.constData(), XPLMFindPluginsMenu(), xpMenuContainterIndex, XpMenu::menuHandlerInternal, nullptr);

  int idx = 0;
  int menuIndex = XPLMAppendMenuItem(p->xpMenuId, versionName.constData(), static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, xplm_Menu_NoCheck);
  XPLMEnableMenuItem(p->xpMenuId, menuIndex, 0);
  p->items[idx] = {VERSION, menuIndex, this};
  idx++;

  XPLMAppendMenuSeparator(p->xpMenuId);

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch AI", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchAi ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_AI, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Load AI Aircraft Information", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchAiAircraftInfo ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_AI_INFO, menuIndex, this};
  idx++;

  XPLMAppendMenuSeparator(p->xpMenuId);

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 50 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs <= 75 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_50, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 100 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs > 75 && fetchRateMs <= 125 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_100, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 150 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs > 125 && fetchRateMs <= 175 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_150, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 200 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs > 175 && fetchRateMs <= 225 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_200, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 250 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs > 225 && fetchRateMs <= 375 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_250, menuIndex, this};
  idx++;

  menuIndex = XPLMAppendMenuItem(p->xpMenuId, "Fetch Rate 500 ms", static_cast<void *>(&p->items[idx]), 1);
  XPLMCheckMenuItem(p->xpMenuId, menuIndex, fetchRateMs > 375 ? xplm_Menu_Checked : xplm_Menu_Unchecked);
  p->items[idx] = {FETCH_RATE_500, menuIndex, this};
  idx++;
}
