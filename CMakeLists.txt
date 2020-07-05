cmake_minimum_required(VERSION 3.5)

# We need a minimum C++ standard to avoid tons of #ifdef's
set(CMAKE_CXX_STANDARD 14)
# We want CMake to complain if our compiler can not provide at least that
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Since this is a cross-platform app, we want to get an error for any compiler specific extensions
set(CMAKE_CXX_EXTENSIONS OFF)

project(einstein)

set(INCLUDEPKGS TestBigEndian FindPkgConfig CheckLibraryExists)
foreach(pname IN LISTS INCLUDEPKGS)
    include(${pname})
endforeach()

option(portaudio "PortAudio output support" OFF)
option(nativeportaudio "PortAudio (native library) support" OFF)
option(pulseaudio "PulseAudio output support" ON)
option(armlejit "Use ARMLE JIT" OFF)
option(appX11 "X11+CLI application" ON)
option(appFLTK "FLTK application" OFF)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

# Release / Debug compile flags.
set(_FLAGS_DEBUG "-g -O0 -DDEBUG -DDEBUG_SOUND -DPA_ENABLE_DEBUG_OUTPUT")
set(_FLAGS_RELEASE "-O3 -DRELEASE")

set(CMAKE_CXX_FLAGS_RELEASE "${_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${_FLAGS_DEBUG}")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wwrite-strings -Wno-multichar -Wfloat-equal -Wshadow -Wpointer-arith -Wconversion")
endif()

set(LINKLIBS)

find_library(M_LIB m)
list(APPEND LINKLIBS ${M_LIB})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    set(CMAKE_THREAD_PREFER_PTHREAD)
    find_package(Threads)
    if (Threads_FOUND)
        list(APPEND LINKLIBS pthread)
    else()
        message(FATAL_ERROR "Missing library: pthread")
    endif()


    find_library(FFI_LIB ffi)
    find_library(DL_LIB dl)
    list(APPEND LINKLIBS ${DL_LIB} ${PTHREAD_LIB} ${FFI_LIB})
endif()

TEST_BIG_ENDIAN(_IS_BIG_ENDIAN)
add_definitions(-DTARGET_RT_BIG_ENDIAN=$<BOOL:${_IS_BIG_ENDIAN}> -DTARGET_RT_LITTLE_ENDIAN=$<NOT:${_IS_BIG_ENDIAN}>)

# K Libs
add_subdirectory(K)

###########################
# Einstein library code

## 1) Gather all common sources

file(GLOB elib_common_sources "Emulator/*.cp")
# file(GLOB elib_Monitor_sources "Monitor/*.cp")

set_source_files_properties(${elib_common_sources} PROPERTIES LANGUAGE CXX)
# set_source_files_properties(${elib_Monitor_sources} PROPERTIES LANGUAGE CXX)

# Add "einsteinlib" as a library target
add_library(einsteinlib STATIC ${elib_common_sources})

# add include directories ... do I need this?
target_include_directories(einsteinlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:einsteinlib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/Emulator
    $<INSTALL_INTERFACE:einsteinlib>/Emulator
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/Emulator/JIT/Generic
    $<INSTALL_INTERFACE:einsteinlib>/Emulator/JIT/Generic
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/Monitor/
    $<INSTALL_INTERFACE:einsteinlib>/Monitor/
)

# Platform-specific stuff
if(${WIN32})
    target_sources(einsteinlib PUBLIC "Emulator/Win32/CompatibilityWin32.cp")
    set(EINSTEIN_TARGET "WIN32")
    set(EINSTEIN_SCREEN_MANAGER "FL")

    target_include_directories(einsteinlib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/Emulator/Win32
        $<INSTALL_INTERFACE:einsteinlib>/Emulator/Win32
    )
    set_source_files_properties("Emulator/Win32/CompatibilityWin32.cp" PROPERTIES LANGUAGE CXX)
elseif(${UNIX})

    if (${APPLE})
        # FIXME OS X is called OPENSTEP ...
        set(EINSTEIN_TARGET "OPENSTEP")
        # FIXME add OS X sources here.
        set(EINSTEIN_SCREEN_MANAGER "Cocoa")
    else()
        if (appX11)
            set(EINSTEIN_SCREEN_MANAGER "X11")
        endif()

        if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "BSD")
            set(EINSTEIN_TARGET "BSD")

        else()
            set(EINSTEIN_TARGET "LINUX")
        endif()
    endif()
endif()

#
# ---- Einstein Source Files -------------------------------------------------------------
#

set ( sources
	Emulator/Files/TFileManager.cpp
	Emulator/Files/TFileManager.h
	Emulator/Host/THostInfo.cpp
	Emulator/Host/THostInfo.h
	Emulator/Host/UserInfoDefinitions.h
	Emulator/JIT/Generic/IncludeMask.h
	Emulator/JIT/Generic/IncludeMaskRm.h
	Emulator/JIT/Generic/IncludeModeRd.h
	Emulator/JIT/Generic/IncludeModeRn.h
	Emulator/JIT/Generic/IncludeModeRnRd.h
	Emulator/JIT/Generic/IncludeRd.h
	Emulator/JIT/Generic/IncludeRdRm.h
	Emulator/JIT/Generic/IncludeRdRnRsRm15.h
	Emulator/JIT/Generic/IncludeRdRsRm15.h
	Emulator/JIT/Generic/IncludeRm.h
	Emulator/JIT/Generic/IncludeRm15.h
	Emulator/JIT/Generic/IncludeRn.h
	Emulator/JIT/Generic/IncludeRn15.h
	Emulator/JIT/Generic/IncludeRnRd.h
	Emulator/JIT/Generic/IncludeRnRdRm.h
	Emulator/JIT/Generic/IncludeRnRsRm15.h
	Emulator/JIT/Generic/IncludeRsRm15.h
	Emulator/JIT/Generic/RepeatMacros.h
	Emulator/JIT/Generic/TJITGeneric.cpp
	Emulator/JIT/Generic/TJITGeneric.h
	Emulator/JIT/Generic/TJITGenericPage.cpp
	Emulator/JIT/Generic/TJITGenericPage.h
	Emulator/JIT/Generic/TJITGenericROMPatch.cpp
	Emulator/JIT/Generic/TJITGenericROMPatch.h
	Emulator/JIT/Generic/TJITGeneric_BlockDataTransfer.cpp
	Emulator/JIT/Generic/TJITGeneric_BlockDataTransfer.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_ArithmeticOp.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_ArithmeticOp.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_ArithmeticOp_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_LogicalOp.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_LogicalOp.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_LogicalOp_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MRS.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MRS.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MRS_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MSR.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MSR.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MSR_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MoveOp.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MoveOp.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_MoveOp_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_TestOp.cpp
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_TestOp.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_TestOp_template.h
	Emulator/JIT/Generic/TJITGeneric_DataProcessingPSRTransfer_common.h
	Emulator/JIT/Generic/TJITGeneric_LDM1_template.h
	Emulator/JIT/Generic/TJITGeneric_LDM2_template.h
	Emulator/JIT/Generic/TJITGeneric_LDM3_template.h
	Emulator/JIT/Generic/TJITGeneric_Macros.h
	Emulator/JIT/Generic/TJITGeneric_Multiply.cpp
	Emulator/JIT/Generic/TJITGeneric_Multiply.h
	Emulator/JIT/Generic/TJITGeneric_MultiplyAndAccumulate.cpp
	Emulator/JIT/Generic/TJITGeneric_MultiplyAndAccumulate.h
	Emulator/JIT/Generic/TJITGeneric_Multiply_template.h
	Emulator/JIT/Generic/TJITGeneric_Other.cpp
	Emulator/JIT/Generic/TJITGeneric_Other.h
	Emulator/JIT/Generic/TJITGeneric_STM1_template.h
	Emulator/JIT/Generic/TJITGeneric_STM2_template.h
	Emulator/JIT/Generic/TJITGeneric_SingleDataSwap.cpp
	Emulator/JIT/Generic/TJITGeneric_SingleDataSwap.h
	Emulator/JIT/Generic/TJITGeneric_SingleDataSwap_template.h
	Emulator/JIT/Generic/TJITGeneric_SingleDataTransfer.cpp
	Emulator/JIT/Generic/TJITGeneric_SingleDataTransfer.h
	Emulator/JIT/Generic/TJITGeneric_SingleDataTransfer_template.h
	Emulator/JIT/Generic/TJITGeneric_Test.cpp
	Emulator/JIT/Generic/TJITGeneric_Test.h
	Emulator/JIT/Generic/TJITGeneric_Test_template.h
	Emulator/JIT/JIT.h
	Emulator/JIT/TJIT.h
	Emulator/JIT/TJITCache.cpp
	Emulator/JIT/TJITCache.h
	Emulator/JIT/TJITPage.cpp
	Emulator/JIT/TJITPage.h
	Emulator/JIT/TJITPerformance.cpp
	Emulator/JIT/TJITPerformance.h
	Emulator/Log/TBufferLog.cpp
	Emulator/Log/TBufferLog.h
	Emulator/Log/TFileLog.cpp
	Emulator/Log/TFileLog.h
	Emulator/Log/TLog.cpp
	Emulator/Log/TLog.h
	Emulator/Log/TRAMLog.cpp
	Emulator/Log/TRAMLog.h
	Emulator/Log/TStdOutLog.cpp
	Emulator/Log/TStdOutLog.h
	Emulator/NativeCalls/TVirtualizedCalls.cpp
	Emulator/NativeCalls/TVirtualizedCalls.h
	Emulator/NativeCalls/TVirtualizedCallsPatches.cpp
	Emulator/NativeCalls/TVirtualizedCallsPatches.h
	Emulator/Serial/TTcpClientSerialPortManager.cpp
	Emulator/Serial/TTcpClientSerialPortManager.h
	Emulator/Network/TNetworkManager.cpp
	Emulator/Network/TNetworkManager.h
	Emulator/PCMCIA/TATACard.cpp
	Emulator/PCMCIA/TATACard.h
	Emulator/PCMCIA/TLinearCard.cpp
	Emulator/PCMCIA/TLinearCard.h
	Emulator/PCMCIA/TNE2000Card.cpp
	Emulator/PCMCIA/TNE2000Card.h
	Emulator/PCMCIA/TPCMCIACard.cpp
	Emulator/PCMCIA/TPCMCIACard.h
	Emulator/PCMCIA/TPCMCIAController.cpp
	Emulator/PCMCIA/TPCMCIAController.h
	Emulator/Platform/PlatformEvents.h
	Emulator/Platform/PlatformGestalt.h
	Emulator/Platform/TNewt.cpp
	Emulator/Platform/TNewt.h
	Emulator/Platform/TPlatformManager.cpp
	Emulator/Platform/TPlatformManager.h
	Emulator/ROM/TAIFFile.cpp
	Emulator/ROM/TAIFFile.h
	Emulator/ROM/TAIFROMImageWithREXes.cpp
	Emulator/ROM/TAIFROMImageWithREXes.h
	Emulator/ROM/TFlatROMImageWithREX.cpp
	Emulator/ROM/TFlatROMImageWithREX.h
	Emulator/ROM/TROMImage.cpp
	Emulator/ROM/TROMImage.h
	Emulator/Screen/TFLScreenManager.cpp
	Emulator/Screen/TFLScreenManager.h
	Emulator/Screen/TNullScreenManager.cpp
	Emulator/Screen/TNullScreenManager.h
	Emulator/Screen/TScreenManager.cpp
	Emulator/Screen/TScreenManager.h
	Emulator/Serial/TBasicSerialPortManager.cpp
	Emulator/Serial/TBasicSerialPortManager.h
	Emulator/Serial/TSerialPortManager.cpp
	Emulator/Serial/TSerialPortManager.h
	Emulator/Serial/TSerialPorts.cpp
	Emulator/Serial/TSerialPorts.h
	Emulator/Serial/TSerialHostPort.h
	Emulator/Serial/TSerialHostPortDirect.cpp
	Emulator/Serial/TSerialHostPortDirect.h
	Emulator/Serial/TSerialHostPortPTY.cpp
	Emulator/Serial/TSerialHostPortPTY.h
	Emulator/Sound/TBufferedSoundManager.cpp
	Emulator/Sound/TBufferedSoundManager.h
	Emulator/Sound/TNullSoundManager.cpp
	Emulator/Sound/TNullSoundManager.h
	Emulator/Sound/TSoundManager.cpp
	Emulator/Sound/TSoundManager.h
	Emulator/TARMProcessor.cpp
	Emulator/TARMProcessor.h
	Emulator/TDMAManager.cpp
	Emulator/TDMAManager.h
	Emulator/TEmulator.cpp
	Emulator/TEmulator.h
	Emulator/TFlash.cpp
	Emulator/TFlash.h
	Emulator/THashMapCache.h
	Emulator/TInterruptManager.cpp
	Emulator/TInterruptManager.h
	Emulator/TMMU.cpp
	Emulator/TMMU.h
	Emulator/TMemory.cpp
	Emulator/TMemory.h
	Emulator/TMemoryConsts.h
	Emulator/TNativePrimitives.cpp
	Emulator/TNativePrimitives.h
	K/Defines/KDefinitions.cpp
	K/Defines/KDefinitions.h
	K/Defines/UByteSex.h
	K/Exceptions/Errors/TError.cpp
	K/Exceptions/Errors/TError.h
	K/Exceptions/Errors/TMemError.cpp
	K/Exceptions/Errors/TMemError.h
	K/Exceptions/IO/TEOFException.cpp
	K/Exceptions/IO/TEOFException.h
	K/Exceptions/IO/TIOException.cpp
	K/Exceptions/IO/TIOException.h
	K/Exceptions/TException.cpp
	K/Exceptions/TException.h
	K/Misc/CRC32.h
	K/Misc/CRC32.cpp
	K/Misc/RelocHack.h
	K/Misc/TCircleBuffer.cpp
	K/Misc/TCircleBuffer.h
	K/Misc/TDoubleLinkedList.h
	K/Misc/TMappedFile.cpp
	K/Misc/TMappedFile.h
	K/Streams/TFileStream.cpp
	K/Streams/TFileStream.h
	K/Streams/TRandomAccessStream.h
	K/Streams/TStream.cpp
	K/Streams/TStream.h
	K/Threads/TCondVar.cpp
	K/Threads/TCondVar.h
	K/Threads/TMutex.cpp
	K/Threads/TMutex.h
	K/Threads/TThread.cpp
	K/Threads/TThread.h
	K/Unicode/UUTF16CStr.cpp
	K/Unicode/UUTF16CStr.h
	K/Unicode/UUTF16Conv.cpp
	K/Unicode/UUTF16Conv.h
	K/Unicode/UnicodeDefinitions.h
	Monitor/TFLMonitor.cpp
	Monitor/TFLMonitor.h
	Monitor/TMonitor.cpp
	Monitor/TMonitor.h
	Monitor/TMonitorCore.cpp
	Monitor/TMonitorCore.h
	Monitor/TSymbolList.cpp
	Monitor/TSymbolList.h
	Monitor/UDisasm.cpp
	Monitor/UDisasm.h
	app/TFLApp.cpp
	app/TFLApp.h
	app/TFLAppUI.cpp
	app/TFLAppUI.h
	app/TFLAppUI.fl
	app/TFLSettingsUI.fl
	app/TFLSettingsUI.cpp
	app/TFLSettingsUI.h
	app/TFLSettings.cpp
	app/TFLSettings.h
	app/TFLAppWindow.cpp
	app/TFLAppWindow.h
	app/TPathHelper.cpp
	app/TPathHelper.h
	app/Version.h
	app/Version.h.in
	Drivers/EinsteinRex.cpp
	Drivers/EinsteinRex.h
)

if (portaudio OR nativeportaudio)
    add_definitions("-DAUDIO_PORTAUDIO=1")
endif()

if (pulseaudio)
    add_definitions("-DAUDIO_PULSEAUDIO=1")
endif()

# Add the Emulator/ sub-dir, which has CMakeLists all the way down
add_subdirectory(Emulator)
add_subdirectory(Monitor)

install(TARGETS einsteinlib DESTINATION lib/${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_BUILD_TYPE})

target_link_libraries(einsteinlib monitor files host jit log nativecalls network pcmcia platform rom screen serial sound ${LINKLIBS} k)


############################
# EINSTEIN EMULATOR
add_executable(einstein app/einstein.cp app/TCLIApp.cp)

install(TARGETS einstein
DESTINATION bin/${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_BUILD_TYPE})

set_source_files_properties(app/einstein.cp app/TCLIApp.cp PROPERTIES LANGUAGE CXX)
target_link_libraries(einstein k einsteinlib)
