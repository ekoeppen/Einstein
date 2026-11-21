#
# Add source files required to build Einstein.
#

list ( APPEND cmake_sources
	Emulator/Screen/CMakeLists.txt
)

list ( APPEND common_sources
	Emulator/Screen/TNullScreenManager.cpp
	Emulator/Screen/TNullScreenManager.h
	Emulator/Screen/TScreenManager.cpp
	Emulator/Screen/TScreenManager.h
)

if ( EINSTEIN_FLTK_FRONTEND )

list ( APPEND app_sources
	Emulator/Screen/TFLScreenManager.cpp
	Emulator/Screen/TFLScreenManager.h
)

endif ()

if ( EINSTEIN_SDL3_FRONTEND )

list ( APPEND app_sources
	Emulator/Screen/TSDLScreenManager.cpp
	Emulator/Screen/TSDLScreenManager.h
)

endif ()

if ( EINSTEIN_COCOA_FRONTEND )

list ( APPEND app_sources
	Emulator/Screen/CocoaEmulatorApp.h
	Emulator/Screen/CocoaScreenProxy.h
	Emulator/Screen/CocoaScreenProxy.mm
	Emulator/Screen/TCocoaPowerButtonNSView.h
	Emulator/Screen/TCocoaPowerButtonNSView.mm
	Emulator/Screen/TCocoaScreenGlue.h
	Emulator/Screen/TCocoaScreenGlue.mm
	Emulator/Screen/TCocoaScreenManager.h
	Emulator/Screen/TCocoaScreenManager.mm
	Emulator/Screen/TCocoaScreenView.h
	Emulator/Screen/TCocoaScreenView.mm
)

endif ()
