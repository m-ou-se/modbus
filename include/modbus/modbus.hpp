#pragma once

#include <chrono>
#include <cstdint>

#include <mstd/error_or.hpp>
#include <mstd/range.hpp>

#include "error.hpp"

namespace Modbus {

using mstd::range;
using mstd::error_or;

using byte_t = unsigned char;
using uint16_t = std::uint16_t;

class Modbus {

public:

	// The timeout is the time to wait for the first byte, the full response
	// may take longer. With timeout == 0, the functions immediately return
	// Error::timeout, without waiting for any response.
	using timeout_t = std::chrono::milliseconds;

	// Function code 0x01.
	error_or<void> read_coils(
		byte_t slave_id,
		uint16_t address,
		range<bool> values,
		timeout_t timeout
	);

	// Same, with different types instead of bool.
	error_or<void> read_coils(byte_t, uint16_t, range<unsigned char>, timeout_t);
	error_or<void> read_coils(byte_t, uint16_t, range<uint16_t>, timeout_t);

	// Function code 0x02.
	error_or<void> read_inputs(
		byte_t slave_id,
		uint16_t address,
		range<bool> values,
		timeout_t timeout
	);

	// Same, with different types instead of bool.
	error_or<void> read_inputs(byte_t, uint16_t, range<unsigned char>, timeout_t);
	error_or<void> read_inputs(byte_t, uint16_t, range<uint16_t>, timeout_t);

	// Function code 0x03.
	error_or<void> read_holding_registers(
		byte_t slave_id,
		uint16_t address,
		range<uint16_t> values,
		timeout_t timeout
	);

	// Function code 0x04.
	error_or<void> read_input_registers(
		byte_t slave_id,
		uint16_t address,
		range<uint16_t> values,
		timeout_t timeout
	);

	// Function code 0x05.
	error_or<void> write_single_coil(
		byte_t slave_id,
		uint16_t address,
		bool value,
		timeout_t timeout
	);

	// Function code 0x06.
	error_or<void> write_single_register(
		byte_t slave_id,
		uint16_t address,
		uint16_t value,
		timeout_t timeout
	);

	// Function code 0x0F.
	error_or<void> write_multiple_coils(
		byte_t slave_id,
		uint16_t address,
		range<bool const> values,
		timeout_t timeout
	);

	// Same, with different types instead of bool.
	error_or<void> write_multiple_coils(byte_t, uint16_t, range<unsigned char const>, timeout_t);
	error_or<void> write_multiple_coils(byte_t, uint16_t, range<uint16_t const>, timeout_t);

	// Function code 0x05 or 0x0F.
	error_or<void> write_coils(
		byte_t slave_id,
		uint16_t address,
		range<bool const> values,
		timeout_t timeout
	) {
		if (values.size() == 1) return write_single_coil(slave_id, address, values[0], timeout);
		else return write_multiple_coils(slave_id, address, values, timeout);
	}
	error_or<void> write_coils(
		byte_t slave_id,
		uint16_t address,
		range<unsigned char const> values,
		timeout_t timeout
	) {
		if (values.size() == 1) return write_single_coil(slave_id, address, values[0], timeout);
		else return write_multiple_coils(slave_id, address, values, timeout);
	}
	error_or<void> write_coils(
		byte_t slave_id,
		uint16_t address,
		range<uint16_t const> values,
		timeout_t timeout
	) {
		if (values.size() == 1) return write_single_coil(slave_id, address, values[0], timeout);
		else return write_multiple_coils(slave_id, address, values, timeout);
	}

	// Function code 0x10.
	error_or<void> write_multiple_registers(
		byte_t slave_id,
		uint16_t address,
		range<uint16_t const> values,
		timeout_t timeout
	);

	// Function code 0x06 or 0x10.
	error_or<void> write_registers(
		byte_t slave_id,
		uint16_t address,
		range<uint16_t const> values,
		timeout_t timeout
	) {
		if (values.size() == 1) return write_single_register(slave_id, address, values[0], timeout);
		else return write_multiple_registers(slave_id, address, values, timeout);
	}

	struct read_file_group {
		uint16_t file_number;
		uint16_t address;
		range<uint16_t> data;
	};

	// Function code 0x14.
	error_or<void> read_file_record(
		byte_t slave_id,
		range<read_file_group> groups,
		timeout_t timeout
	);

	struct write_file_group {
		uint16_t file_number;
		uint16_t address;
		range<uint16_t const> data;
	};

	// Function code 0x15.
	error_or<void> write_file_record(
		byte_t slave_id,
		range<write_file_group> groups,
		timeout_t timeout
	);

	// Function code 0x16.
	error_or<void> mask_write_register(
		byte_t slave_id,
		uint16_t address,
		uint16_t and_mask,
		uint16_t or_mask,
		timeout_t timeout
	);

	// Function code 0x17.
	error_or<void> read_write_registers(
		byte_t slave_id,
		uint16_t write_address,
		range<uint16_t const> write_values,
		uint16_t read_address,
		range<uint16_t> read_values,
		timeout_t timeout
	);

	// Send a raw command.
	// If the result does not fit into response_buffer, Error::invalid_response
	// is returned. On success, the subrange (starting at the first byte) of
	// response_buffer that contains the response is returned. parameters and
	// response_buffer may overlap. The response does not include the function
	// code. Exception responses are returned as the corresponding Error.
	virtual error_or<range<byte_t>> raw_command(
		byte_t slave_id,
		byte_t function_code,
		range<byte_t const> parameters,
		range<byte_t> response_buffer,
		timeout_t timeout
	) = 0;

};

}
