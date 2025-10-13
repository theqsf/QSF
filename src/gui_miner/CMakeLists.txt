# QSF Quantum-Safe Miner GUI
# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# Find Qt5 with required components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 is required to build the GUI miner")
endif()

# Find ZMQ library - use find_library instead of pkg-config
find_library(ZMQ_LIB zmq)
find_path(ZMQ_INCLUDE_PATH zmq.h)

if(NOT ZMQ_LIB)
    message(FATAL_ERROR "ZMQ library is required to build the GUI miner")
endif()

if(NOT ZMQ_INCLUDE_PATH)
    message(FATAL_ERROR "ZMQ headers are required to build the GUI miner")
endif()

message(STATUS "Qt5 found: ${Qt5_DIR}")
message(STATUS "Qt5 Core: ${Qt5Core_DIR}")
message(STATUS "Qt5 Widgets: ${Qt5Widgets_DIR}")
message(STATUS "Qt5 Network: ${Qt5Network_DIR}")
message(STATUS "ZMQ found: ${ZMQ_LIB}")
message(STATUS "ZMQ include: ${ZMQ_INCLUDE_PATH}")

# Set Qt variables
set(QT_VERSION "Qt5")
set(QT_PREFIX "Qt5")

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

# Link libraries - add back required dependencies
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

# Enable automoc for Qt
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

# Windows-specific icon handling
if(WIN32)
    # Create Windows resource file for the icon
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.ico"
        "${CMAKE_CURRENT_BINARY_DIR}/qsf_icon.ico"
        COPYONLY
    )
    
    # Copy icon next to the executable in your current build output
    add_custom_command(TARGET qsf-gui-miner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/icons/qsf_icon.ico"
            "$<TARGET_FILE_DIR:qsf-gui-miner>/qsf_icon.ico"
        COMMENT "Copying Windows icon"
    )
endif()

# Copy required DLLs for distribution
if(WIN32)
    # Create a custom target to copy DLLs and daemon executable
    add_custom_target(copy_gui_miner_dlls ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${Qt5_DIR}/../../../bin/Qt5Core.dll"
            "${Qt5_DIR}/../../../bin/Qt5Gui.dll"
            "${Qt5_DIR}/../../../bin/Qt5Widgets.dll"
            "${Qt5_DIR}/../../../bin/Qt5Network.dll"
            "${CMAKE_BINARY_DIR}/bin/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "/mingw64/bin/libgcc_s_seh-1.dll"
            "/mingw64/bin/libstdc++-6.dll"
            "/mingw64/bin/libwinpthread-1.dll"
            "/mingw64/bin/libdouble-conversion.dll"
            "/mingw64/bin/libicudt77.dll"
            "/mingw64/bin/libicuin77.dll"
            "/mingw64/bin/libicuuc77.dll"
            "/mingw64/bin/libpcre2-16-0.dll"
            "/mingw64/bin/libmd4c.dll"
            "${CMAKE_BINARY_DIR}/bin/"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "/mingw64/share/qt5/plugins/platforms/qwindows.dll"
            "/mingw64/share/qt5/plugins/platforms/qdirect2d.dll"
            "/mingw64/share/qt5/plugins/platforms/qminimal.dll"
            "/mingw64/share/qt5/plugins/platforms/qoffscreen.dll"
            "${CMAKE_BINARY_DIR}/bin/platforms/"
        # Note: Daemon executable (qsf.exe) is built separately and should already be in bin/
        COMMENT "Copying GUI miner dependencies"
    )
    
    # Make the GUI miner depend on the DLL copying
    # add_dependencies(qsf-gui-miner copy_gui_miner_dlls)
endif()

# Print configuration info
message(STATUS "Building QSF GUI Miner with ${QT_VERSION}")
message(STATUS "Qt include dirs: ${Qt5_INCLUDE_DIRS}")
message(STATUS "Qt libraries: ${Qt5_LIBRARIES}")
message(STATUS "ZMQ include dir: ${ZMQ_INCLUDE_PATH}")
message(STATUS "ZMQ library: ${ZMQ_LIB}") 