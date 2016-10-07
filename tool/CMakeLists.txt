cmake_minimum_required(VERSION 3.1)

project(modbus-tool)

add_executable(modbus-tool
	tool.cpp
)

set_target_properties(modbus-tool PROPERTIES OUTPUT_NAME modbus)

target_link_libraries(modbus-tool PUBLIC modbus-serial-rtu)
