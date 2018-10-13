cmake_minimum_required(VERSION 3.0)

#project name
project(ble-lib)

set(CMAKE_BIULD_TYPE Debug)

include_directories(
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
)

aux_source_directory(${PROJECT_SOURCE_DIR} BLE_LIB_SRC)

set(PI_BLE_SOURCES ${BLE_LIB_SRC})

add_library(ble-lib STATIC ${PI_BLE_SOURCES})
target_link_libraries(ble-lib)