# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

# Minimum required CMake version
cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

set(projectName "EXRcheck")
set(appName "EXRcheckApp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist")	# set where CMake should put .exe output (current: out/build/{CONFIG_NAME}/dist/ )

set(assetsRelPath "assets")												# relative (project-) path to "assets" directory (storing files, images etc...)
set(testImagesRelPath "${assetsRelPath}/images/test")					# relative (project-) path to image assets
set(sourceAssetsAbsPath "${CMAKE_SOURCE_DIR}/${assetsRelPath}")			# absolute (system-) path to source assets
set(sourceTestImagesAbsPath "${CMAKE_SOURCE_DIR}/${testImagesRelPath}")	# absolute (system-) path to source image assets

project(${projectName})

# Set C++ standard
set(CMAKE_CXX_STANDARD 20) 				# set number of C++ standard project uses
set(CMAKE_CXX_STANDARD_REQUIRED True) 	# prohibit older standards

# Add include directory
include_directories(${CMAKE_SOURCE_DIR}/src)	# CMAKE_SOURCE_DIR is the directory where your top-level CMakeLists.txt is located (i.e., ProjectDirectory).

# Specify target executable
add_executable(${appName}
	"src/Application.cpp"	# also possible: "src/*.cpp"
	"src/ExeParams.h"
)


# "dist"" folder - is a folder that, after building process, contains a shippable application with all its dependencies and stuff for user.
# Post-build
set(exeDir "$<TARGET_FILE_DIR:${appName}>")			# After add_exe(..) only! Resolves at build-time to where {appName}.exe is placed.

# 1. Copy test assets to "dist" folder
set(assetsDistPath "${exeDir}/${assetsRelPath}")	# resolves to where target .exe is placed + "/assets"
# Note:
# As of 2025.04.18,
# Debug		build requires file(s) located in ${SOURCE_DIR}/images (so-called, assets, resources)
# Release	build DOES NOT require file(s) any assets, but put images next to .exe for testing comfort
#
# copy assets directory (${SOURCE_DIR}/assets) folder and its content to the folder where built .exe was placed (only Debug build)
#if (CMAKE_BUILD_TYPE STREQUAL "Debug")					# check if build type is "Debug"
	add_custom_command(TARGET ${appName} POST_BUILD		# custom command, attached to ${appName} target, and run after .exe is compiled and linked OK. 
		COMMAND ${CMAKE_COMMAND} -E copy_directory		# ${CMAKE_COMMAND} returns full path to cmake executable; -E = helper command, not build.
			"${sourceAssetsAbsPath}"					# source folder (copy from).
			"${assetsDistPath}"							# destination folder (copy to).
		COMMENT "CMakeLists.txt : ${CMAKE_BUILD_TYPE} build: copy assets to target .exe output folder ( ${assetsDistPath} )"		# shown in terminal during build process.
	)
#endif()

# 2. Copy content of ProjectDir/_docs/USER_MANUAL_SHIPPABLE.txt to "dist/docs/{appName}_ReadMe.txt"
set(sourceReadmePath "${CMAKE_SOURCE_DIR}/_docs/USER_README_SHIPPABLE.txt")	# Path to source readme
# make "docs" directory
add_custom_command(
    TARGET ${appName}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${exeDir}/docs"
    COMMENT "CMakeLists.txt : Make directory 'docs' at target .exe output folder "
)
# Copy user manual to shippable file
add_custom_command(
    TARGET ${appName}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${sourceReadmePath}"
        "${exeDir}/docs/${appName}_ReadMe.txt"		# copy sourceManual.txt to {exeDir}/docs/{appName}_ReadMe.txt
    COMMENT "CMakeLists.txt : Copy user readme to output 'dist' folder at target .exe output folder"
)
