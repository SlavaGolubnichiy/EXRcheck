# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

# Minimum required CMake version
cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

set(projectName "EXRcheck")
set(appName "EXRcheck_App")

set(assetsRelPath "assets")												# relative (project-) path to "assets" directory (storing files, images etc...)
set(testImagesRelPath "${assetsRelPath}/images/test")					# relative (project-) path to image assets
set(sourceAssetsAbsPath "${CMAKE_SOURCE_DIR}/${assetsRelPath}")			# absolute (system-) path to source assets
set(sourceTestImagesAbsPath "${CMAKE_SOURCE_DIR}/${testImagesRelPath}")	# absolute (system-) path to source image assets

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist")	# set where CMake should put .exe output (current: out/build/{CONFIG_NAME}/dist/ )

project(${projectName})

# Set C++ standard
set(CMAKE_CXX_STANDARD 20) 				# set number of C++ standard project uses
set(CMAKE_CXX_STANDARD_REQUIRED True) 	# prohibit older standards

# Add include directory
include_directories(${CMAKE_SOURCE_DIR}/src)	# CMAKE_SOURCE_DIR is the directory where your top-level CMakeLists.txt is located (i.e., ProjectDirectory).

# Specify target executable
add_executable(${appName}
	"src/Application.cpp"	# also possible: "src/*.cpp"
)

# Note:
# As of 2025.04.18,
# Debug		build requires file(s) located in ${SOURCE_DIR}/images (so-called, assets, resources)
# Release	build DOES NOT require file(s) any assets, but put images next to .exe for testing comfort
#
# copy assets directory (${SOURCE_DIR}/assets) folder and its content to the folder where built .exe was placed (only Debug build)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")							# check if build type is "Debug"
	add_custom_command(TARGET ${appName} POST_BUILD				# custom command, attached to ${appName} target, and run after .exe is compiled and linked OK. 
		COMMAND ${CMAKE_COMMAND} -E copy_directory				# ${CMAKE_COMMAND} returns full path to cmake executable; -E = helper command, not build.
			"${sourceAssetsAbsPath}"							# source folder (copy from).
			"$<TARGET_FILE_DIR:${appName}>/${assetsRelPath}"	# destination folder (copy to). Resolves at build-time to where built {appName}.exe is placed + "/images".
		COMMENT "${CMAKE_BUILD_TYPE} build: copy assets to .exe output directory - DONE."		# shown in terminal during build process.
	)
endif()

# copy images from (${SOURCE_DIR}/assets/images/test) folder and its content to the folder where built .exe was placed (only Release build)
if (CMAKE_BUILD_TYPE STREQUAL "Release")				# check the build type
	add_custom_command(TARGET ${appName} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_SOURCE_DIR}/${testImagesRelPath}"	# source folder (copy from).
			"$<TARGET_FILE_DIR:${appName}>"				# destination folder (copy to). Resolves at build-time to where built {appName}.exe is placed.
		COMMENT "${CMAKE_BUILD_TYPE} build: copy assets to .exe output directory - DONE."
	)
endif()
