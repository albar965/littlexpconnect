Little XpConnect
================

A free and open source X-Plane plugin that allows to use Little Navmap as a moving map when flying.
Little Navmap can connect locally or remote to this plugin which runs its own TCP network server.

Little Navmap is a free open source flight planner, navigation tool, moving map, airport search and
airport information system for Flight Simulator X, Prepar3D and X-Plane.

This plugin is only useful for X-plane and is available for Windows, macOS and Linux (all 64 bit only).

|Windows, Linux or Mac Machine       |         Windows, Linux or Mac Machines  |
--------------------------------------------------------------------------------
| X-Plane <-> Little XpConnect <- (Local)   -> Little Navmap on Computer 1
                               <- (Network) -> Little Navmap on Computer 2

------------------------------------------------------------------------------
-- INSTALLATION --------------------------------------------------------------
------------------------------------------------------------------------------

Copy the whole plugin directory "Little XpConnect" into the plugins directory within your Resources directory
in the X-Plane installation.
The complete path should look like:
.../X-Plane 11/Resources/plugins/Little XpConnect

You can check the X-Plane plugin manager to see if it is loaded  correctly.

When connecting with Little Navmap use the computer name where X-Plane is running and
the default port 51968. Use "localhost" if X-Plane and Little Navmap run on the same computer.

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
DefaultPort=51968     : This is the default port to connect to. Change this if it collides with another program.
FetchAiAircraft=true  : Also get AI aircraft (not available yet)
FetchRate=200         : Data will be fetched from X-Plane every 200 milliseconds.
                        Lower this value to get a more fluid movement of the aircraft symbol on the map.
                        Increase this value if you think it causes performance problems.
UpdateRate=500        : Data will be sent over the network ever 500 milliseconds.
                        Lower this value to get a more fluid movement of the aircraft symbol on the map.
                        Increase this value if you have a slow network.

------------------------------------------------------------------------------
-- LICENSE -------------------------------------------------------------------
------------------------------------------------------------------------------

This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/littlexpconnect

The source code for Little XpConnect is available at Github too:
https://github.com/albar965/littlexpconnect

Copyright 2015-2017 Alexander Barthel (albar965@mailbox.org).

