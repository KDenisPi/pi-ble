cmake_minimum_required(VERSION 3.0)

#project name
project(pi-ble)

set(THIRD_PARTY_HOME "${CMAKE_SOURCE_DIR}/../third-party")
set(PI_LIBRARY_HOME "${CMAKE_SOURCE_DIR}/../pi-robot")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_BIULD_TYPE Debug)

#cmake version 3.0 and above
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_library(PI_UTILS_LIB pi-utils PATH ${PI_LIBRARY_HOME}/build/pi-utils)
message( STATUS "PI_UTILS_LIB is ${PI_UTILS_LIB}")

include_directories(BEFORE
    ${PI_LIBRARY_HOME}/pi-utils/
)

#C++ 11
#add_cxx_flags( "-std=c++11" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -std=c++11 -fexceptions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO} -std=c++11 -fexceptions")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_library(libpi-utils STATIC IMPORTED)
set(EXTRA_LIBS ${EXTRA_LIBS})

add_subdirectory(${PROJECT_SOURCE_DIR}/ble-lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-client)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-server)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-ftp)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-ftp-client)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-ftp-server-test)





