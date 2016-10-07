#include <array>
#include <chrono>
#include <cstdint>

#include <mstd/error_or.hpp>
#include <mstd/range.hpp>

#include <modbus/error.hpp>
#include <modbus/modbus.hpp>

namespace Modbus {

error_or<void> Modbus::read_coils(
	byte_t slave_id,
	uint16_t address,
	range<bool> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 2000) return std::error_code(Error::request_too_large);
	std::array<byte_t, 251> buffer;
	byte_t * p = buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	size_t n_expected_bytes = (values.size() + 7) / 8 + 1;
	if (auto r = raw_command(slave_id, 0x01, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || buffer[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		for (size_t i = 0; i < values.size(); ++i) {
			values[i] = buffer[1 + i / 8] >> i % 8 & 1;
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::read_inputs(
	byte_t slave_id,
	uint16_t address,
	range<bool> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 2000) return std::error_code(Error::request_too_large);
	std::array<byte_t, 251> buffer;
	byte_t * p = buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	size_t n_expected_bytes = (values.size() + 7) / 8 + 1;
	if (auto r = raw_command(slave_id, 0x02, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || buffer[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		for (size_t i = 0; i < values.size(); ++i) {
			values[i] = buffer[1 + i / 8] >> i % 8 & 1;
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::read_holding_registers(
	byte_t slave_id,
	uint16_t address,
	range<uint16_t> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 125) return std::error_code(Error::request_too_large);
	std::array<byte_t, 251> buffer;
	byte_t * p = buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	size_t n_expected_bytes = values.size() * 2 + 1;
	if (auto r = raw_command(slave_id, 0x03, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || buffer[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		for (size_t i = 0; i < values.size(); ++i) {
			values[i] = uint16_t(buffer[1 + i * 2]) << 8 | buffer[2 + i * 2];
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::read_input_registers(
	byte_t slave_id,
	uint16_t address,
	range<uint16_t> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 125) return std::error_code(Error::request_too_large);
	std::array<byte_t, 251> buffer;
	byte_t * p = buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	size_t n_expected_bytes = values.size() * 2 + 1;
	if (auto r = raw_command(slave_id, 0x04, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || buffer[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		for (size_t i = 0; i < values.size(); ++i) {
			values[i] = uint16_t(buffer[1 + i * 2]) << 8 | buffer[2 + i * 2];
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::write_single_coil(
	byte_t slave_id,
	uint16_t address,
	bool value,
	std::chrono::milliseconds timeout
) {
	std::array<byte_t, 4> request = {{
		byte_t(address >> 8),
		byte_t(address & 0xFF),
		byte_t(value ? 0xFF : 0x00),
		0x00
	}};
	std::array<byte_t, 4> response;
	if (auto r = raw_command(slave_id, 0x05, request, response, timeout)) {
		if (r->size() != request.size() || response != request) {
			return std::error_code(Error::invalid_response);
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::write_single_register(
	byte_t slave_id,
	uint16_t address,
	uint16_t value,
	std::chrono::milliseconds timeout
) {
	std::array<byte_t, 4> request = {{
		byte_t(address >> 8),
		byte_t(address & 0xFF),
		byte_t(value >> 8),
		byte_t(value & 0xFF)
	}};
	std::array<byte_t, 4> response;
	if (auto r = raw_command(slave_id, 0x06, request, response, timeout)) {
		if (*r != request) return std::error_code(Error::invalid_response);
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::write_multiple_coils(
	byte_t slave_id,
	uint16_t address,
	range<bool const> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 1968) return std::error_code(Error::request_too_large);
	byte_t n_data_bytes = (values.size() + 7) / 8;
	std::array<byte_t, 251> request_buffer;
	byte_t * p = request_buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	*p++ = n_data_bytes;
	for (size_t i = 0; i < values.size(); ++i) {
		if (values[i]) request_buffer[5 + i / 8] |= 1 << i % 8;
	}
	std::array<byte_t, 4> response;
	if (auto r = raw_command(slave_id, 0x0F, {request_buffer.data(), p + n_data_bytes}, response, timeout)) {
		if (*r != range<byte_t>(request_buffer.data(), 4)) {
			return std::error_code(Error::invalid_response);
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::write_multiple_registers(
	byte_t slave_id,
	uint16_t address,
	range<uint16_t const> values,
	std::chrono::milliseconds timeout
) {
	if (values.size() > 123) return std::error_code(Error::request_too_large);
	std::array<byte_t, 251> request_buffer;
	byte_t * p = request_buffer.data();
	*p++ = address >> 8;
	*p++ = address & 0xFF;
	*p++ = values.size() >> 8;
	*p++ = values.size() & 0xFF;
	*p++ = values.size() * 2;
	for (uint16_t v : values) {
		*p++ = v >> 8;
		*p++ = v & 0xFF;
	}
	std::array<byte_t, 4> response;
	if (auto r = raw_command(slave_id, 0x10, {request_buffer.data(), p}, response, timeout)) {
		if (*r != range<byte_t>(request_buffer.data(), 4)) {
			return std::error_code(Error::invalid_response);
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::read_file_record(
	byte_t slave_id,
	range<read_file_group> groups,
	std::chrono::milliseconds timeout
) {
	if (groups.size() > 35) return std::error_code(Error::request_too_large);
	size_t n_expected_bytes = 1;
	for (auto const & g : groups) {
		n_expected_bytes += g.data.size() * 2 + 2;
		if (n_expected_bytes > 251) return std::error_code(Error::request_too_large);
	}
	std::array<byte_t, 251> buffer;
	buffer[0] = groups.size() * 7;
	byte_t * p = &buffer[1];
	for (auto const & g : groups) {
		*p++ = 0x06;
		*p++ = g.file_number >> 8;
		*p++ = g.file_number & 0xFF;
		*p++ = g.address >> 8;
		*p++ = g.address & 0xFF;
		*p++ = g.data.size() >> 8;
		*p++ = g.data.size() & 0xFF;
	}
	if (auto r = raw_command(slave_id, 0x14, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || (*r)[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		p = &buffer[1];
		for (auto const & g : groups) {
			if (*p++ != 1 + g.data.size() * 2 || *p++ != 0x06) {
				return std::error_code(Error::invalid_response);
			}
			for (uint16_t & v : g.data) {
				uint16_t high = *p++;
				v = high << 8 | *p++;
			}
		}
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::write_file_record(
	byte_t slave_id,
	range<write_file_group> groups,
	std::chrono::milliseconds timeout
) {
	size_t n_bytes = 1;
	for (auto const & g : groups) {
		n_bytes += g.data.size() * 2 + 7;
		if (n_bytes > 251) return std::error_code(Error::request_too_large);
	}
	std::array<byte_t, 251> request_buffer;
	byte_t * p = request_buffer.data();
	*p++ = n_bytes - 1;
	for (auto const & g : groups) {
		*p++ = 0x06;
		*p++ = g.file_number >> 8;
		*p++ = g.file_number & 0xFF;
		*p++ = g.address >> 8;
		*p++ = g.address & 0xFF;
		*p++ = g.data.size() >> 8;
		*p++ = g.data.size() & 0xFF;
		for (uint16_t v : g.data) {
			*p++ = v >> 8;
			*p++ = v & 0xFF;
		}
	}
	range<byte_t> request(request_buffer.data(), p);
	std::array<byte_t, 251> response_buffer;
	if (auto r = raw_command(slave_id, 0x15, request, {response_buffer.data(), request.size()}, timeout)) {
		if (*r != request) return std::error_code(Error::invalid_response);
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::mask_write_register(
	byte_t slave_id,
	uint16_t address,
	uint16_t and_mask,
	uint16_t or_mask,
	std::chrono::milliseconds timeout
) {
	std::array<byte_t, 6> request = {{
		byte_t(address >> 8),
		byte_t(address & 0xFF),
		byte_t(and_mask >> 8),
		byte_t(and_mask & 0xFF),
		byte_t(or_mask >> 8),
		byte_t(or_mask & 0xFF)
	}};
	std::array<byte_t, 6> response;
	if (auto r = raw_command(slave_id, 0x16, request, response, timeout)) {
		if (*r != request) return std::error_code(Error::invalid_response);
		return {};
	} else {
		return r.error();
	}
}

error_or<void> Modbus::read_write_registers(
	byte_t slave_id,
	uint16_t write_address,
	range<uint16_t const> write_values,
	uint16_t read_address,
	range<uint16_t> read_values,
	std::chrono::milliseconds timeout
) {
	if (read_values.size() > 125 || write_values.size() > 121) {
		return std::error_code(Error::request_too_large);
	}
	std::array<byte_t, 251> buffer;
	byte_t * p = buffer.data();
	*p++ = read_address >> 8;
	*p++ = read_address & 0xFF;
	*p++ = read_values.size() >> 8;
	*p++ = read_values.size() & 0xFF;
	*p++ = write_address >> 8;
	*p++ = write_address & 0xFF;
	*p++ = write_values.size() >> 8;
	*p++ = write_values.size() & 0xFF;
	*p++ = write_values.size() * 2;
	for (uint16_t v : read_values) {
		*p++ = v >> 8;
		*p++ = v & 0xFF;
	}
	size_t n_expected_bytes = read_values.size() * 2 + 1;
	if (auto r = raw_command(slave_id, 0x17, {buffer.data(), p}, {buffer.data(), n_expected_bytes}, timeout)) {
		if (r->size() != n_expected_bytes || (*r)[0] != n_expected_bytes - 1) {
			return std::error_code(Error::invalid_response);
		}
		p = &buffer[1];
		for (uint16_t & v : read_values) {
			uint16_t high = *p++;
			v = high << 8 | *p++;
		}
		return {};
	} else {
		return r.error();
	}
}

}
