# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

# Minimum required CMake version
#cmake_minimum_required (VERSION 3.30.5 FATAL_ERROR)

set(projectName "EXRcheck")
project(${projectName})
set(versionMajor "0")
set(versionMinor "1")
set(versionPatch "000")
# define configuration type (x64 or x86)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(archName "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(archName "x86")
else()
    message(FATAL_ERROR "Unsupported architecture: CMAKE_SIZEOF_VOID_P is not 4 or 8, it is ${CMAKE_SIZEOF_VOID_P}. Architecture check MUST be AFTER project(${projectName}).")
endif()
set(appName "EXRcheckApp_${archName}") # _${versionMajor}.${versionMinor}.${versionPatch}_${configName}")

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")	# check if build type is "Debug"
#	set(buildType "debug")
#endif()

# set where CMake should put .exe output (current: out/build/{CONFIG_NAME}/dist/EXRcheckApp_0.1.000_x86 )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/${appName}_${versionMajor}.${versionMinor}.${versionPatch}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20) 				# set number of C++ standard project uses
set(CMAKE_CXX_STANDARD_REQUIRED True) 	# prohibit older standards

# Add include directory
include_directories(${CMAKE_SOURCE_DIR}/src)	# CMAKE_SOURCE_DIR is the directory where your top-level CMakeLists.txt is located (i.e., ProjectDirectory).

# Specify target executable
add_executable(${appName}
	"src/Application.cpp"	# also possible: "src/*.cpp"
)



############################################################################################################
##############################			post-build				############################################
############################################################################################################

# "dist"" folder - is a folder that, after building process, contains a shippable application with all its dependencies and stuff for user.
# Post-build
set(exeDir "$<TARGET_FILE_DIR:${appName}>")			# After add_exe(..) only! Resolves at build-time to where {appName}.exe is placed.

# 1. Copy content of ProjectDir/_docs/USER_MANUAL_SHIPPABLE.txt to "dist/docs/{appName}_ReadMe.txt"
set(userDocsSourceDir "_docs/user")
set(userDocsDestDir "docs")
set(userDocsSourceAbsPath "${CMAKE_SOURCE_DIR}/${userDocsSourceDir}")	# path to source of docs for end-user
set(userDocsDestAbsPath "${exeDir}/${userDocsDestDir}")					# path to destination of docs for end-user (for this build)
# make "docs" directory
add_custom_command(
    TARGET ${appName}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${userDocsDestAbsPath}"
    COMMENT "CMakeLists.txt : Make directory '${userDocsDestDir}' at target .exe output folder"
)
# copy user manual to shippable file
add_custom_command(
    TARGET ${appName}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory	# copy all content of specified source directory
        "${userDocsSourceAbsPath}"					# copy user docs from projectDir/_docs/user/
        "${userDocsDestAbsPath}"					# copy user docs to ${exeDir}/docs/
    COMMENT "CMakeLists.txt : Copy content of 'ProjectDir/${userDocsSourceDir}/' to target .exe output folder '/${userDocsDestDir}' "
)

# 2. Copy only test image assets to target .exe folder
set(assetsDir "assets")											# relative (project-) path to image assets
set(assetsSourceAbsPath "${CMAKE_SOURCE_DIR}/${assetsDir}")		# absolute (system-) path to source assets
set(assetsDestAbsPath "${exeDir}/${assetsDir}")					# absolute (system-) path to where .exe is placed
# Note:
# As of 2025.04.18,
# Debug		build requires file(s) located in ${SOURCE_DIR}/images (so-called, assets, resources)
# Release	build DOES NOT require file(s) any assets, but put images next to .exe for testing comfort
#
# copy assets directory (${SOURCE_DIR}/assets) folder and its content to the folder where built .exe was placed (only Debug build)
add_custom_command(TARGET ${appName} POST_BUILD		# custom command, attached to ${appName} target, and run after .exe is compiled and linked OK. 
	COMMAND ${CMAKE_COMMAND} -E copy_directory		# ${CMAKE_COMMAND} returns full path to cmake executable; -E = helper command, not build.
		"${assetsSourceAbsPath}"					# source folder (copy from).
		"${assetsDestAbsPath}"						# destination folder (copy to).
	COMMENT "CMakeLists.txt : ${CMAKE_BUILD_TYPE} build: copy assets from 'projectDir/${assetsDir}' to 'target .exe output folder /${assetsDir}' "		# shows up in terminal during build process
)
