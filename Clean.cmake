# cleanup.cmake

# README!
# This script searches for files/directories listed inside
# the foreach() loop below, and if found - deletes them.
# Thus, cleaning the project directory from CMake and IDE build-/test-related files.
# Script prints the result using colored terminal output if supported.

#set(COLOR_GREEN "\033[0;32m")
#set(COLOR_BLUE "\033[0;34m")
#set(COLOR_RESET "\033[0m")

message("Cleaning up CMake cache and build files...")

# List of files and directories to delete
set(FILES "
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
	
")

# Split into list
string(REPLACE "\n" ";" TO_DELETE_LIST "${FILES}")

foreach(item IN LISTS TO_DELETE_LIST)
    string(STRIP "${item}" item)  # Remove any leading/trailing whitespace
    if(item STREQUAL "")
        continue()
    endif()

    file(GLOB MATCHED_ITEMS "${item}")

    if(MATCHED_ITEMS)
        foreach(path IN LISTS MATCHED_ITEMS)
            file(REMOVE_RECURSE "${path}")
            message("___ removed : ${item}")
        endforeach()
    else()
        # message("___ not found ${item}")
    endif()
endforeach()

message("Cleanup completed.")
