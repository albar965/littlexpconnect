#!/bin/bash

# Used commands in this script
#
# "sudo" is a command that allows you to run other commands as
# administrator. Therefore, it asks for your password. The command "xattr" changes
# attributes. The flag "-r" instructs xattr to change all subfolders too and the flag
# "-d com.apple.quarantine" tells it to delete the quarantine flag.

echo
echo
echo ========================================================================

# First change current working folder to the script location and print it
cd "$(dirname "${BASH_SOURCE[0]}")"
echo Running in folder:
pwd

# Check for plugin file
if [ ! -f "mac.xpl" ]; then
  echo
  echo Could not find the X-Plane plugin file mac.xpl.
  echo You have to run this script from inside the Little Xpconnect folder.
  echo
  echo ========================================================================
  echo
  echo
else
  echo
  echo This script fixes the attributes of the Little Xpconnect X-Plane plugin
  echo which is needed on macOS Catalina or newer.
  echo
  echo Run this after each update or installation of Little Xpconnect.
  echo
  echo You have to enter your password in the next step since administrative
  echo priviledges are required to fix the attributes.
  echo
  echo Note that the password is not echoed on the terminal and
  echo the key does not move or change otherwise while entering the password.
  echo This is normal.
  echo
  # Change attribute for whole folder including this script
  sudo xattr -r -d com.apple.quarantine *
  if [ $? -eq 0 ];
  then
    echo
    echo Done. You should now see the plugin in the X-Plane plugin manager and
    echo should also be able to connect Little Navmap with X-Plane.
  else
    echo
    echo ERROR. Failed to change attributes.
  fi
  echo
  echo ========================================================================
  echo
  echo
fi


