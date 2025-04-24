@echo off
setlocal EnableDelayedExpansion

REM Check if running on Windows
if not "%OS%"=="Windows_NT" (
	echo This script can not run on non-Windows OS
	pause
	exit /b 1
)

REM Double-check using `ver`, useful for edge cases
ver | findstr /i "Windows" >nul
if errorlevel 1 (
	echo This script can not run on non-Windows OS
	pause
	exit /b 1
)

REM If here, we're running on Windows
REM Place your actual Windows-only commands here

REM README!
REM This script searches for files/directories listed inside 
REM 	for %%F in ( <here> ) do ...
REM and if found - deletes them. Thus, cleaning the project directory 
REM from CMake and IDE build-/test-/else- related files (unrelated to project payload). 
REM Script prints the result of searching (and deleting) each file/directory using 
REM printColored function (using powershell call to print colored text sustaining script compatibility).

REM 1. Possible improvement: iterate through ".gitignore" file and delete files/directories listed 
REM there to avoid duplicating the list of temporary files thus improving project maintenance.

echo Cleaning up CMake cache and build files...

REM List of files and directories to delete (v1)
:: set FILES= bin ^
::	build ^
::	out ^
::	CMakeFiles ^
::	cmake_install.cmake ^
::	compile_commands.json ^
::	CMakeCache.txt ^
::	CTestTestfile.cmake ^
::	CMakeSettings.json ^
::	Makefile

REM Loop through each item
REM for %%F in (%FILES%) do ( 	// v1
for %%F in (
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
	
) do (
	REM Expand wildcards by checking if the directory or file exists
	if exist "%%F" (
		rd /s /q "%%F" 2>nul || del /f /q "%%F" >nul
		call :printStatus "___ %%F " "removed" "Green"
REM    ) else (
REM		call :printStatus "___ %%F " "not found" "DarkBlue"
	)
)

call :printColored "Green" "Cleanup completed."

pause
endlocal
exit /b



:printColored
REM %~1 = color, %~2 = filename
powershell -Command "Write-Host '%~2' -ForegroundColor %~1"
exit /b

:printStatus
REM %1 = filename, %2 = status message, %3 = color
<nul set /p="%~1"
call :printColored "%~3" "%~2"
exit /b
