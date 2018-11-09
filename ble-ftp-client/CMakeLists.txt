cmake_minimum_required(VERSION 3.0)

#project name
project(ble-ftp-client)

set(VER_MJR 0)
set(VER_MIN 1)

set(CMAKE_BIULD_TYPE Debug)
find_library(PI_UTILS_LIB pi-utils PATH ${PI_LIBRARY_HOME}/build/pi-utils)
message( STATUS "PI_UTILS_LIB is ${PI_UTILS_LIB}")

set(EXTRA_LIBS ${EXTRA_LIBS} ble-ftp ble-lib ${PI_UTILS_LIB} bluetooth pthread)
message( STATUS "EXTRA_LIBS is ${EXTRA_LIBS}")

include_directories(BEFORE
    ${PROJECT_SOURCE_DIR}/../ble-ftp
    ${PROJECT_SOURCE_DIR}/../ble-lib
)

aux_source_directory(${PROJECT_SOURCE_DIR} BLE_FTP_CLIENT_SOURCES)
add_executable(bleftpclient ${BLE_FTP_CLIENT_SOURCES})
target_link_libraries(bleftpclient ${EXTRA_LIBS})
