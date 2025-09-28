# QSF Quantum-Safe Miner GUI
# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# Find Qt5 with required components
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 is required to build the GUI miner")
endif()

# Find ZMQ library
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)
if(NOT ZMQ_FOUND)
    message(FATAL_ERROR "ZMQ library is required to build the GUI miner")
endif()

message(STATUS "Qt5 found: ${Qt5_DIR}")
message(STATUS "Qt5 Core: ${Qt5Core_DIR}")
message(STATUS "Qt5 Widgets: ${Qt5Widgets_DIR}")
message(STATUS "Qt5 Network: ${Qt5Network_DIR}")
message(STATUS "ZMQ found: ${ZMQ_LIBRARIES}")

# Set Qt variables
set(QT_VERSION "Qt5")
set(QT_PREFIX "Qt5")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/crypto
    ${CMAKE_SOURCE_DIR}/external/randomx/src
    ${CMAKE_SOURCE_DIR}/external/qrcodegen
    ${ZMQ_INCLUDE_DIRS}
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
    ${ZMQ_LIBRARIES}
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

# Print configuration info
message(STATUS "Building QSF GUI Miner with ${QT_VERSION}")
message(STATUS "Qt include dirs: ${Qt5_INCLUDE_DIRS}")
message(STATUS "Qt libraries: ${Qt5_LIBRARIES}")
message(STATUS "ZMQ include dirs: ${ZMQ_INCLUDE_DIRS}")
message(STATUS "ZMQ libraries: ${ZMQ_LIBRARIES}") 