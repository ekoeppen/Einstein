#
# At this time we only support CMake builds for the FLTK variant of Einstein.
#

if ( EINSTEIN_FLTK_FRONTEND )
	include ( app/FLTK/CMakeLists.txt )
endif ()

if ( EINSTEIN_SDL3_FRONTEND )
	include ( app/SDL/CMakeLists.txt )
endif ()

if ( EINSTEIN_COCOA_FRONTEND )
	list ( APPEND app_sources
		app/einstein.mm
		app/TCocoaAppController.h
		app/TCocoaAppController.mm
		app/TCocoaFileManager.h
		app/TCocoaFileManager.mm
		app/TCocoaListenerWindow.h
		app/TCocoaListenerWindow.mm
		app/TCocoaMonitorController.h
		app/TCocoaMonitorController.mm
		app/TCocoaROMDumperController.h
		app/TCocoaROMDumperController.mm
		app/TCocoaSetupController.h
		app/TCocoaSetupController.mm
		app/TCocoaUserDefaults.h
		app/Scripting/EinsteinDoNewtonScriptCommand.h
		app/Scripting/EinsteinDoNewtonScriptCommand.mm
		app/Scripting/EinsteinInstallPackageCommand.h
		app/Scripting/EinsteinInstallPackageCommand.mm
	)
endif ()

list ( APPEND cmake_sources
	app/CMakeLists.txt
)

list ( APPEND common_sources
	app/TPathHelper.cpp
	app/TPathHelper.h
)

list ( APPEND common_sources
	app/Version.h
	app/Version_CMake.h.in
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	list (APPEND common_sources
		app/TCocoaPathHelper.mm
		app/TCocoaPathHelper.h
	)
endif()
