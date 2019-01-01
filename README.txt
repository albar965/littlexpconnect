Little Xpconnect
================

A free and open source X-Plane plugin that allows to use Little Navmap as a
moving map when flying. Little Navmap can connect locally to this plugin.
Remote or networked setups can be done with the Little Navconnect program.

Little Navmap is a free open source flight planner, navigation tool, moving
map, airport search and airport information system for Flight Simulator X,
Prepar3D and X-Plane.

This plugin is only useful for X-Plane and is available for Windows, macOS and
Linux (all 64 bit only). Only tested on X-Plane 11.

Local connection scenario:

|Windows, Linux or Mac Machine      |    Windows, Linux or Mac Machine  |
-------------------------------------------------------------------------
X-Plane <-> Little Xpconnect  <- (Local) -> Little Navmap


Network scenario for X-Plane using the Little Xpconnect plugin and the Little
Navonnect agent:

|Windows, Linux or Mac Machine                           |     Windows, Linux or Mac Machines  |
------------------------------------------------------------------------------------------------
X-Plane <-> Little XpConnect <-> Little Navonnect <- (Network) -> Little Navmap on Computer 1
                                                  <- (Network) -> Little Navmap on Computer 2

------------------------------------------------------------------------------
-- INSTALLATION --------------------------------------------------------------
------------------------------------------------------------------------------

Download the correct archive for your operating system. The program is bundled
with Little Navmap and can be downloaded separately too.

Delete any old "Little Xpconnect" installations in the plugins directory before copying
the new version.
Do not merge new and old installtions.

Copy the whole plugin directory "Little Xpconnect" into the directory "plugins"
in the directory "Resources" in the X-Plane installation. The complete path
should look like:
.../X-Plane 11/Resources/plugins/Little Xpconnect

You can check the X-Plane plugin manager to see if it is loaded  correctly.
The installation was successful if "Little Xpconnect" shows up in the plugin manager.

Note that "Little Xpconnect" does not add menu items in the X-Plane "Plugins" menu.

When connecting with Little Navmap choose the "Connect directly to a local
X-Plane simulator" option. See the Little Navmap manual for more information.

------------------------------------------------------------------------------
-- FILES ---------------------------------------------------------------------
------------------------------------------------------------------------------

The Plugin creates a log file which can be found in
* "C:\Users\YOURUSERNAME\AppData\Local\Temp\abarthel-little_xpconnect.log" for Windows,
* "/tmp/abarthel-little_xpconnect.log" for Linux and
* "/var/folders/RANDOMIZED_DIRECTORY_NAME/abarthel-little_xpconnect.log" for macOS.

Additionally a small settings file is created which can be found in
* "C:\Users\YOURUSERNAME\AppData\Roaming\ABarthel\little_xpconnect.ini" for Windows,
* "$HOME/.config/ABarthel/little_xpconnect.ini" for Linux and macOS.

------------------------------------------------------------------------------
-- CONFIGURATION -------------------------------------------------------------
------------------------------------------------------------------------------

Settings can only be changed using the configuration file
"little_xpconnect.ini".

[Options]
FetchAiAircraft=true  : Also get AI aircraft. You can disable this if
                        you think the plugin causes performance problems.
FetchRate=200         : Data will be fetched from X-Plane every 200
                        milliseconds.
                        Lower this value to get a more fluid movement of the
                        aircraft symbol on the map. Increase this value if you
                        think the plugin causes performance problems.

------------------------------------------------------------------------------
-- LICENSE -------------------------------------------------------------------
------------------------------------------------------------------------------

This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/littlexpconnect

The source code for Little Xpconnect is available at Github too:
https://github.com/albar965/littlexpconnect

Copyright 2015-2019 Alexander Barthel (albar965@mailbox.org).

