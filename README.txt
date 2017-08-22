Little Xpconnect
================

A free and open source X-Plane plugin that allows to use Little Navmap as a moving map when flying.
Little Navmap can connect locally to this plugin. Remote or networked setups can be done with
the Little Navconnect program.

Little Navmap is a free open source flight planner, navigation tool, moving map, airport search and
airport information system for Flight Simulator X, Prepar3D and X-Plane.

This plugin is only useful for X-Plane and is available for Windows, macOS and Linux (all 64 bit only).
Only tested on X-Plane 11.

|Windows, Linux or Mac Machine        |         Windows, Linux or Mac Machine  |
--------------------------------------------------------------------------------
| X-Plane <-> Little Xpconnect  <- (Local)   -> Little Navmap

------------------------------------------------------------------------------
-- INSTALLATION --------------------------------------------------------------
------------------------------------------------------------------------------

Copy the whole plugin directory "Little Xpconnect" into the plugins directory within your Resources directory
in the X-Plane installation.
The complete path should look like:
.../X-Plane 11/Resources/plugins/Little Xpconnect

You can check the X-Plane plugin manager to see if it is loaded  correctly.

When connecting with Little Navmap use the computer name where X-Plane is running and
the default port 51968. Use "localhost" if X-Plane and Little Navmap run on the same computer.

The installation on Linux and macOS computers is simlar except different paths.

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

Settings can only be changed using the configuration file "little_xpconnect.ini".

[Options]
FetchAiAircraft=true  : Also get AI aircraft. You can disable this if
                        you think the plugin causes performance problems.
FetchRate=200         : Data will be fetched from X-Plane every 200 milliseconds.
                        Lower this value to get a more fluid movement of the aircraft symbol on the map.
                        Increase this value if you think the plugin causes performance problems.

------------------------------------------------------------------------------
-- LICENSE -------------------------------------------------------------------
------------------------------------------------------------------------------

This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/littlexpconnect

The source code for Little Xpconnect is available at Github too:
https://github.com/albar965/littlexpconnect

Copyright 2015-2017 Alexander Barthel (albar965@mailbox.org).

