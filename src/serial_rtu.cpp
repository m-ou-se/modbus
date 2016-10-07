#include <chrono>

#include <modbus/crc.hpp>
#include <modbus/error.hpp>
#include <modbus/modbus.hpp>
#include <modbus/serial_rtu.hpp>

using namespace std::chrono_literals;

namespace Modbus {

error_or<range<byte_t>> ModbusSerialRtu::raw_command(
	byte_t slave_id,
	byte_t function_code,
	range<byte_t const> parameters,
	range<byte_t> response_buffer,
	std::chrono::milliseconds timeout
) {
	{
		uint16_t crc = crc_ibm().add(slave_id).add(function_code).add(parameters).get();
		unsigned char crc_low = crc & 0xFF;
		unsigned char crc_high = crc >> 8;

		range<byte_t const> to_write[] = {
			slave_id, function_code, parameters, crc_low, crc_high
		};

		for (auto r : to_write) {
			for (byte_t b : r) {
				if (auto e = port_.write(b).error()) return e;
			}
		}
	}

	if (timeout.count() == 0) {
		// With timeout == 0, we don't expect any response at all.
		// (For example, for a broadcast command.)
		return std::error_code(Error::timeout);
	}

	crc_ibm crc;

	bool is_invalid_response = false;
	bool is_exception_response = false;
	unsigned char exception_code;

	auto read = port_.read(timeout);
	size_t read_i = 0;

	for (; read.ok() && read.value(); read = port_.read(2ms), ++read_i) {
		crc.add(*read.value());
		if (read_i == 0) {
			if (read.value() != slave_id) is_invalid_response = true;
		} else if (read_i == 1) {
			if (*read.value() == (function_code | 0x80)) {
				is_exception_response = true;
				response_buffer = exception_code;
			} else if (read.value() != function_code) {
				is_invalid_response = true;
			}
		} else if (read_i >= 2 && read_i < 2 + response_buffer.size()) {
			response_buffer[read_i - 2] = *read.value();
		} else if (read_i > 255) {
			// Modbus serial RTU frames may be no longer than 256 bytes.
			// (1 byte slave id, 2 bytes crc, and 253 PDU.)
			return std::error_code(Error::bad_frame);
		} else if (read_i >= 4 + response_buffer.size()) {
			// Response larger than what fits in response_buffer.
			is_invalid_response = true;
		}
	}

	if (read.error()) return read.error();

	if (read_i == 0) {
		// No bytes were read before the first timeout.
		return std::error_code(Error::timeout);
	}

	if (read_i < 4) {
		// Any valid modbus message is at least four bytes.
		return std::error_code(Error::bad_frame);
	}

	if (crc.get() != 0) {
		return std::error_code(Error::bad_crc);
	}

	if (is_invalid_response || (is_exception_response && read_i != 5)) {
		return std::error_code(Error::invalid_response);
	}

	if (is_exception_response) {
		return std::error_code(Error(exception_code));
	}

	return response_buffer.subrange(0, read_i - 4);
}

}
