#pragma once

#include <chrono>
#include <cstdint>

#include <mstd/error_or.hpp>
#include <mstd/range.hpp>
#include <serial/serial.hpp>

#include "modbus.hpp"

namespace Modbus {

class ModbusSerialRtu : public Modbus {

private:
	Serial::Port port_;

public:
	explicit ModbusSerialRtu(Serial::Port port)
		: port_(std::move(port)) {}

	Serial::Port & port() { return port_; }

	error_or<range<byte_t>> raw_command(
		byte_t slave_id,
		byte_t function_code,
		range<byte_t const> parameters,
		range<byte_t> response_buffer,
		std::chrono::milliseconds timeout
	) override;

};

}
