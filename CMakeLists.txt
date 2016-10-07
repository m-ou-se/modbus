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

target_include_directories(modbus PUBLIC
	include
)

target_link_libraries(modbus PUBLIC
	mstd
)

add_library(modbus-serial-rtu
	src/crc.cpp
	src/serial_rtu.cpp
)

target_link_libraries(modbus-serial-rtu PUBLIC
	modbus
	serial
)
