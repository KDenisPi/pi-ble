cmake_minimum_required(VERSION 3.0)

#project name
project(ble-ftp)

set(CMAKE_BIULD_TYPE Debug)

include_directories(
    ${PROJECT_SOURCE_DIR}/../ble-lib
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
)

aux_source_directory(${PROJECT_SOURCE_DIR} BLE_FTP_SRC)

set(PI_BLE_FTP_SOURCES ${BLE_FTP_SRC})

add_library(ble-ftp STATIC ${PI_BLE_FTP_SOURCES})
target_link_libraries(ble-ftp)