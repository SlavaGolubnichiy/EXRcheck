#!/bin/bash

# Function to wait for any key press
pause() {
	read -n 1 -s -r -p "Press any key to continue..."
	echo
}

# Function to print colored status
printStatus() {
local file="$1"
local message="$2"
local color="$3"

local colorCode=""
case "$color" in
	Green) colorCode="\033[0;32m" ;;
	Blue|DarkBlue) colorCode="\033[0;34m" ;;
	*) colorCode="" ;;
esac

printf "%s ${colorCode}%s\033[0m\n" "$file" "$message"
}

# Function to print colored status
printStatus() {
local text="$1"
local textColored="$2"
local color="$3"

local colorCode=""
case "$color" in
	# Available colors
	Black) colorCode="\033[0;30m" ;;
	Red) colorCode="\033[0;31m" ;;
	Green) colorCode="\033[0;32m" ;;
	Yellow) colorCode="\033[0;33m" ;;
	Blue) colorCode="\033[0;34m" ;;
	Magenta) colorCode="\033[0;35m" ;;
	Cyan) colorCode="\033[0;36m" ;;
	White) colorCode="\033[0;37m" ;;
	# Bold/Bright versions
	BrightBlack|Gray) colorCode="\033[1;30m" ;;
	BrightRed) colorCode="\033[1;31m" ;;
	BrightGreen|LightGreen) colorCode="\033[1;32m" ;;
	BrightYellow|LightYellow) colorCode="\033[1;33m" ;;
	BrightBlue|LightBlue) colorCode="\033[1;34m" ;;
	BrightMagenta|LightMagenta) colorCode="\033[1;35m" ;;
	BrightCyan|LightCyan) colorCode="\033[1;36m" ;;
	BrightWhite|LightWhite) colorCode="\033[1;37m" ;;
	*) colorCode="" ;;
esac

printf "%s ${colorCode}%s\033[0m\n" "$text" "$textColored"
}


# Check if not Linux or macOS
OS_NAME="$(uname)"
case "$OS_NAME" in
Linux|Darwin|MINGW*|MSYS*|CYGWIN*) ;;
*)
	echo "This script is only for Linux / MacOS / Git Bash / MSYS / Cygwin"
	pause
	exit 1
	;;
esac

# README!
# This script searches for files/directories listed inside 
#   for item in "${FILES[@]}"; do ...
# and if found - deletes them. Thus, cleaning the project directory 
# from CMake and IDE build-/test-/else- related files (unrelated to project payload). 
# Script prints the result of searching (and deleting) each file/directory using 
# printColored function (uses ANSI escape codes for compatibility across terminals).

# 1. Possible improvement: iterate through ".gitignore" file and delete files/directories listed 
# there to avoid duplicating the list of temporary files thus improving project maintenance.

echo "Cleaning up CMake cache and build files..."

# List of files and directories to delete (written line-by-line)
FILES=(
	bin
	build
	out
	cmake_install.cmake
	compile_commands.json
	CMakeCache.txt
	CMakeFiles
	CMakeLists.txt.user
	CMakeScripts
	CMakeSettings.json
	CMakeUserPresets.json
	CTestTestfile.cmake
	Makefile
	Testing

	_deps
	*.log
	*.DS_Store
	install_manifest.txt
	
	.vs
	*.ipch
	*.exe
	*.filters
	*.idb
	*.obj
	*.pdb
	*.sln
	*.vcxproj
	*.vcxproj.*
	*.user
	
	.vscode
	
	.idea
	cmake-build-*
	*.iml
	
	*.xcodeproj
	*.xcuserdata
	*.xcworkspace
	*.xcuserstate
	
	.settings
	.cproject
	.project
	
	obj
	*.autosave
	*.cbp
	*.depend
	*.layout
	*.layout.save
)

# Cleanup loop
for pattern in "${FILES[@]}"; do
matches=( $pattern )  # Shell expansion
if compgen -G "$pattern" > /dev/null; then
	for match in "${matches[@]}"; do
	if [ -d "$match" ]; then
		rm -rf "$match"
	elif [ -f "$match" ]; then
		rm -f "$match"
	fi
	printStatus "___ $match" "removed" "BrightGreen"
	done
#else
#  printStatus "___ $pattern" "not found" "Blue"
fi
done

printStatus "" "Cleanup completed." "BrightGreen"

# Pause (Linux/macOS equivalent)
pause
echo