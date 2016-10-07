cmake_minimum_required(VERSION 3.1)

project(modbus)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/mstd/CMakeLists.txt)
	add_subdirectory(mstd)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/serial/CMakeLists.txt)
	add_subdirectory(serial)
endif()

add_library(modbus
	src/error.cpp
	src/modbus.cpp
)

target_link_libraries(modbus PUBLIC mstd)
target_link_libraries(modbus PUBLIC serial)

target_include_directories(modbus PUBLIC include)
