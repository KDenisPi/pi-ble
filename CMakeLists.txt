cmake_minimum_required(VERSION 3.0)

#project name
project(pi-ble)

set(THIRD_PARTY_HOME "${CMAKE_SOURCE_DIR}/../third-party")

#set(GATTLIB_HOME "${THIRD_PARTY_HOME}/gattlib")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_BIULD_TYPE Debug)

#cmake version 3.0 and above
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#include_directories(BEFORE
#    ${GATTLIB_HOME}/include
#)

#C++ 11
#add_cxx_flags( "-std=c++11" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -std=c++11 -fexceptions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO} -std=c++11 -fexceptions")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#add_library(libwiringPi SHARED IMPORTED)
#set(EXTRA_LIBS ${EXTRA_LIBS} ${MOSQT_LIB} ${MOSQT_LIB_CPP})

add_subdirectory(${PROJECT_SOURCE_DIR}/ble-lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-client)
add_subdirectory(${PROJECT_SOURCE_DIR}/ble-server)





