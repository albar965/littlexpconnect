Little Xpconnect
================

A free and open source X-Plane plugin that allows to use Little Navmap as a
moving map when flying. Little Navmap can connect locally to this plugin.
Remote or networked setups can be done with the Little Navconnect program.

Little Navmap is a free open source flight planner, navigation tool, moving
map, airport search and airport information system for Flight Simulator X,
Prepar3D and X-Plane.

This plugin is only useful for X-Plane and is available for Windows, macOS and
Linux (all 64 bit only). Tested on X-Plane 11 and 12.

Issues can be submitted at https://github.com/albar965/littlenavmap/issues .

Local connection scenario:

|Windows, Linux or Mac Machine      |    Windows, Linux or Mac Machine  |
-------------------------------------------------------------------------
X-Plane <-> Little Xpconnect  <- (Local) -> Little Navmap


Network scenario for X-Plane using the Little Xpconnect plugin and the Little
Navconnect agent:

|Windows, Linux or Mac Machine                           |     Windows, Linux or Mac Machines  |
------------------------------------------------------------------------------------------------
X-Plane <-> Little XpConnect <-> Little Navconnect <- (Network) -> Little Navmap on Computer 1
                                                   <- (Network) -> Little Navmap on Computer 2

------------------------------------------------------------------------------
-- INSTALLATION --------------------------------------------------------------
------------------------------------------------------------------------------

Please refer to the Little Navmap installation instructions for a more detailed description:

https://www.littlenavmap.org/manuals/littlenavmap/release/latest/en/INSTALLATION.html
https://www.littlenavmap.org/manuals/littlenavmap/release/latest/en/XPCONNECT.html

Apple macOS users: Keep in mind that you have to clear the quarantine flag for *Little Xpconnect*
on update or installation. See further below or
https://www.littlenavmap.org/manuals/littlenavmap/release/latest/en/XPCONNECT.html#clearing-the-quarantine-flag-on-macos
for more information.

------------------------------------------------------------------------------

Download the correct archive for your operating system. The program is bundled
with Little Navmap and can be downloaded separately too.

Delete any old "Little Xpconnect" installations in the plugins directory before copying
the new version.
Do not merge new and old installations.

Copy the whole plugin directory "Little Xpconnect" into the directory "plugins"
in the directory "Resources" in the X-Plane installation. The complete path
should look like:
.../X-Plane 11/Resources/plugins/Little Xpconnect

You can check the X-Plane plugin manager to see if it is loaded  correctly.
The installation was successful if "Little Xpconnect" shows up in the plugin manager.

Note that "Little Xpconnect" adds a sub-menu in the X-Plane "Plugins" menu.
See CONFIGURATION below.

When connecting with Little Navmap choose the "Connect directly to a local
X-Plane simulator" option. See the Little Navmap manual for more information.

See online manual for installation instructions:
https://www.littlenavmap.org/manuals/littlenavmap/release/latest/en/INSTALLATION.html

-- Apple macOS ---------------------------------------------------------------

This X-Plane plugin comes in two separate folders for different architectures.

"Little Xpconnect arm64":
   This is for Apple computers having an Apple Silicon or an Intel CPU.
   It supports only newer macOS releases from Mojave 10.14 and later.
   Usable for X-Plane 11 and 12.

"Little Xpconnect x86":
   This is for Apple computers having an Intel CPU. This supports
   older macOS releases from High Sierra 10.13.
   Usable for X-Plane 11 and 12.

Install "Little Xpconnect arm64" if in doubt. See the README.txt files in the
Little Xpconnect folders or the Little Navmap user manual for installation instructions:
https://www.littlenavmap.org/manuals/littlenavmap/release/latest/en/INSTALLATION.html#macos

You have to remove the quarantine flag from the plugin after downloading and extracting it.
Otherwise X-Plane will silently not load the plugin.

The procedure may be slightly different depending on whether you're using Big Sur or Catalina.

1. Right or Ctrl-Click in the Finder on the included script `Prepare Little Xpconnect.command`
   and select `Open With` and `Terminal`.
2. You will see a dialog `Prepare Little Xpconnect.command cannot be opened because it is from
   an unidentified developer.` Click `Ok`.
3. Go to `System Preferences` -> `Security and Privacy`. You will see a message mentioning
   the script `Prepare Little Xpconnect.command`. Click `Open Anyway`.
4. Next a dialog `Are you sure you want to open it?` pops up. Click `Open`.

The script shows a few explantions and asks for your password which allows it to run a
command as an administrator. You can look at the script which is a text file to see what
commands are executed.

You can see "Little Xpconnect" in the X-Plane plugin manager once it was
unlocked and loaded successfully.

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

The plugin adds a sub-menu "Little Xpconnect" to the X-Plane plugins menu which
contains the following menu items:

- "Fetch AI": Enable or disable the fetching of AI aircraft and
  ships (carrier and frigate). Disable AI fetching if you see
  stutters or performance degradation within X-Plane when using the plugin.
- "Load AI Aircraft Information": Xpconnect tries to load additional information
  from related ".acf" files. This is done in background and does not cause stutters
  or hesitations. You can disable this function if you see problems. Only limited
  information is available for AI and multiplayer if disabled.
- "Fetch Rate 50 ms" to "Fetch Rate 500 ms": Xpconnect will get all aircraft
  parameters at this rate. Lower values result in smoother aircraft movement but
  may induce stutters or decrease performance in X-Plane.

The changes are applied immediately and are saved on exit.

Note that the settings are shared between all plugin installations on the same computer.

The settings can also be changed using the configuration file "little_xpconnect.ini".

[Options]
FetchAiAircraft=true     : Same as "Fetch AI" in plugin menu.
FetchAiAircraftInfo=true : Same as "Load AI Aircraft Information" in plugin menu.
FetchRateMs=200          : Data will be fetched from X-Plane every 200
                           milliseconds. Same as "Fetch Rate" in plugin menu.
FetchRate=200            : Old obsolete value. Will be deleted on start.

------------------------------------------------------------------------------
-- LICENSE -------------------------------------------------------------------
------------------------------------------------------------------------------

This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/littlexpconnect

The source code for Little Xpconnect is available at Github too:
https://github.com/albar965/littlexpconnect

Copyright 2015-2020 Alexander Barthel (alex@littlenavmap.org).

