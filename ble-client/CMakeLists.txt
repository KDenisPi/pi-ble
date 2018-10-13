cmake_minimum_required(VERSION 3.0)

#project name
project(ble-client)

set(VER_MJR 0)
set(VER_MIN 1)

set(CMAKE_BIULD_TYPE Debug)
set(EXTRA_LIBS ${EXTRA_LIBS} ble-lib bluetooth)

include_directories(BEFORE ${WIRINGPI_INC}
    ${PROJECT_SOURCE_DIR}/../ble-lib
)

aux_source_directory(${PROJECT_SOURCE_DIR} BLE_CLIENT_SOURCES)
add_executable(bleclient ${BLE_CLIENT_SOURCES})
target_link_libraries(bleclient ${EXTRA_LIBS})
