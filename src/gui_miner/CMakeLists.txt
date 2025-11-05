# QSF Quantum-Safe Miner GUI
# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(QSFGuiMiner LANGUAGES CXX)

# Find Qt5 with required components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 is required to build the GUI miner")
endif()

# Find ZMQ library
find_library(ZMQ_LIB zmq)
find_path(ZMQ_INCLUDE_PATH zmq.h)

if(NOT ZMQ_LIB)
    message(FATAL_ERROR "ZMQ library is required to build the GUI miner")
endif()
if(NOT ZMQ_INCLUDE_PATH)
    message(FATAL_ERROR "ZMQ headers are required to build the GUI miner")
endif()

message(STATUS "Qt5 found: ${Qt5_DIR}")
message(STATUS "ZMQ found: ${ZMQ_LIB}")
message(STATUS "ZMQ include: ${ZMQ_INCLUDE_PATH}")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/crypto
    ${CMAKE_SOURCE_DIR}/external/randomx/src
    ${CMAKE_SOURCE_DIR}/external/qrcodegen
    ${ZMQ_INCLUDE_PATH}
    /mingw64/include
)

# Source files
set(GUI_MINER_SOURCES
    main.cpp
    main_window.cpp
    mining_worker.cpp
    quantum_safe_widget.cpp
    zmq_rpc_client.cpp
    wallet_manager.cpp
    ${CMAKE_SOURCE_DIR}/external/qrcodegen/QrCode.cpp
    resources.qrc
)

set(GUI_MINER_HEADERS
    main_window.h
    mining_worker.h
    quantum_safe_widget.h
    zmq_rpc_client.h
    wallet_manager.h
)

# Create executable
add_executable(qsf-gui-miner
    ${GUI_MINER_SOURCES}
    ${GUI_MINER_HEADERS}
)

# Link libraries
target_link_libraries(qsf-gui-miner
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    wallet_api
    common
    cncrypto
    randomx
    easylogging
    epee
    ${Boost_SYSTEM_LIBRARY}
    ${SODIUM_LIBRARY}
    ${ZMQ_LIB}
)

# Set properties
set_target_properties(qsf-gui-miner PROPERTIES
    OUTPUT_NAME "qsf-gui-miner"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable automoc, autorcc, autouic for Qt
set_target_properties(qsf-gui-miner PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Compiler flags
target_compile_features(qsf-gui-miner PRIVATE cxx_std_17)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(qsf-gui-miner pthread)
endif()

if(APPLE)
    target_link_libraries(qsf-gui-miner "-framework Cocoa")
endif()

# Copy icon to build directory for all platforms
add_custom_command(TARGET qsf-gui-miner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.png"
        "$<TARGET_FILE_DIR:qsf-gui-miner>/qsf_icon.png"
    COMMENT "Copying application icon"
)

# Windows-specific icon handling
if(WIN32)
    # Create Windows resource file for icon embedding
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc" 
        "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.ico\"\n"
        "1 VERSIONINFO\n"
        "FILEVERSION 2,1,2,0\n"
        "PRODUCTVERSION 2,1,2,0\n"
        "FILEFLAGSMASK 0x3fL\n"
        "FILEFLAGS 0x0L\n"
        "FILEOS 0x40004L\n"
        "FILETYPE 0x1L\n"
        "FILESUBTYPE 0x0L\n"
        "BEGIN\n"
        "    BLOCK \"StringFileInfo\"\n"
        "    BEGIN\n"
        "        BLOCK \"040904b0\"\n"
        "        BEGIN\n"
        "            VALUE \"CompanyName\", \"QuantumSafeFoundation (theqsf.org)\"\n"
        "            VALUE \"FileDescription\", \"QSF Quantum-Safe GUI Miner - Quantum Leap Release\"\n"
        "            VALUE \"FileVersion\", \"2.1.2.0\"\n"
        "            VALUE \"InternalName\", \"qsf-gui-miner\"\n"
        "            VALUE \"LegalCopyright\", \"Copyright (c) 2025, QuantumSafeFoundation\"\n"
        "            VALUE \"OriginalFilename\", \"qsf-gui-miner.exe\"\n"
        "            VALUE \"ProductName\", \"QSF Quantum-Safe Miner\"\n"
        "            VALUE \"ProductVersion\", \"2.1.2.0\"\n"
        "            VALUE \"ReleaseName\", \"Quantum Leap\"\n"
        "        END\n"
        "    END\n"
        "    BLOCK \"VarFileInfo\"\n"
        "    BEGIN\n"
        "        VALUE \"Translation\", 0x409, 1200\n"
        "    END\n"
        "END\n"
    )
    
    # Add the resource file to the executable
    target_sources(qsf-gui-miner PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.ico"
        "${CMAKE_CURRENT_BINARY_DIR}/qsf_icon.ico"
        COPYONLY
    )
    add_custom_command(TARGET qsf-gui-miner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.ico"
            "$<TARGET_FILE_DIR:qsf-gui-miner>/qsf_icon.ico"
        COMMENT "Copying Windows icon"
    )
endif()

# -------------------
# DLL Copy Section
# -------------------
if(WIN32)
    # Detect MSYS2 root for reliable pathing
    set(MINGW_PREFIX "$ENV{MINGW_PREFIX}")
    if(NOT MINGW_PREFIX)
        set(MINGW_PREFIX "C:/msys64/mingw64")
    endif()

    # List of required MinGW DLLs
    set(MINGW_DLLS
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
        libdouble-conversion.dll
        libicudt77.dll
        libicuin77.dll
        libicuuc77.dll
        libpcre2-16-0.dll
        libpcre2-32-0.dll
        libpcre2-8-0.dll
        libpcre2-posix-3.dll
        libmd4c.dll
    )

    # List of required Qt5 DLLs
    set(QT5_DLLS
        Qt5Core.dll
        Qt5Gui.dll
        Qt5Widgets.dll
        Qt5Network.dll
    )

    # List of additional Qt5 support DLLs
    set(QT5_SUPPORT_DLLS
        libfreetype-6.dll
        libgraphite2.dll
        libharfbuzz-0.dll
        libintl-8.dll
        libpng16-16.dll
        libzstd.dll
        zlib1.dll
        libglib-2.0-0.dll
        libbrotlicommon.dll
        libbrotlidec.dll
        libbrotlienc.dll
        libbz2-1.dll
        libffi-8.dll
        libiconv-2.dll
    )

    # Copy MinGW DLLs
    foreach(dll ${MINGW_DLLS})
        add_custom_command(TARGET qsf-gui-miner POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_PREFIX}/bin/${dll}"
                "${CMAKE_BINARY_DIR}/bin/"
            COMMENT "Copying ${dll}"
        )
    endforeach()

    # Copy Qt5 DLLs
    foreach(dll ${QT5_DLLS})
        add_custom_command(TARGET qsf-gui-miner POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_PREFIX}/bin/${dll}"
                "${CMAKE_BINARY_DIR}/bin/"
            COMMENT "Copying ${dll}"
        )
    endforeach()

    # Copy Qt5 support DLLs
    foreach(dll ${QT5_SUPPORT_DLLS})
        add_custom_command(TARGET qsf-gui-miner POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_PREFIX}/bin/${dll}"
                "${CMAKE_BINARY_DIR}/bin/"
            COMMENT "Copying ${dll}"
        )
    endforeach()

    # Copy Qt5 platform plugins
    add_custom_command(TARGET qsf-gui-miner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/platforms"
        COMMENT "Creating platforms directory"
    )
    
    add_custom_command(TARGET qsf-gui-miner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_PREFIX}/share/qt5/plugins/platforms/qwindows.dll"
            "${CMAKE_BINARY_DIR}/bin/platforms/"
        COMMENT "Copying Qt5 Windows platform plugin"
    )
    
    add_custom_command(TARGET qsf-gui-miner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_PREFIX}/share/qt5/plugins/platforms/qminimal.dll"
            "${CMAKE_BINARY_DIR}/bin/platforms/"
        COMMENT "Copying Qt5 minimal platform plugin"
    )

    # Note: Using manual DLL copying instead of windeployqt for MSYS2 compatibility
    # windeployqt can cause issues with missing OpenGL DLLs in MSYS2 environment
    message(STATUS "Using manual Qt5 DLL copying for MSYS2 compatibility")
endif()


# Print configuration info
message(STATUS "Building QSF GUI Miner with Qt5")
message(STATUS "Qt include dirs: ${Qt5_INCLUDE_DIRS}")
message(STATUS "Qt libraries: ${Qt5_LIBRARIES}")
message(STATUS "ZMQ include dir: ${ZMQ_INCLUDE_PATH}")
message(STATUS "ZMQ library: ${ZMQ_LIB}")
