#pragma once

#include <string>
#include <system_error>

namespace Modbus {

enum class Error : int {
	illegal_function         = 0x01,
	illegal_data_address     = 0x02,
	illegal_data_value       = 0x03,
	slave_device_failure     = 0x04,
	acknowledge              = 0x05,
	slave_device_busy        = 0x06,
	negative_acknowledge     = 0x07,
	memory_parity_error      = 0x08,
	gateway_path_unavailable = 0x0A,
	gateway_no_response      = 0x0B,
	timeout                  = 0x100,
	request_too_large        = 0x200,
	bad_frame                = 0x301, // ADU too short or too long.
	bad_crc                  = 0x302,
	invalid_response         = 0x303, // CRC was ok.
};

class ErrorCategory : public std::error_category {
public:
	char const * name() const noexcept override { return "modbus"; }
	std::string message(int condition) const override {
		switch (Error(condition)) {
			case Error::illegal_function:         return "illegal function";
			case Error::illegal_data_address:     return "illegal data address";
			case Error::illegal_data_value:       return "illegal data value";
			case Error::slave_device_failure:     return "slave device failure";
			case Error::acknowledge:              return "acknowledge";
			case Error::slave_device_busy:        return "slave device busy";
			case Error::negative_acknowledge:     return "negative acknowledge";
			case Error::memory_parity_error:      return "memory parity error";
			case Error::gateway_path_unavailable: return "gateway path unavailable";
			case Error::gateway_no_response:      return "gateway no response";
			case Error::timeout:                  return "timeout";
			case Error::request_too_large:        return "request too large";
			case Error::bad_frame:                return "bad frame";
			case Error::bad_crc:                  return "bad crc";
			case Error::invalid_response:         return "invalid response";
		}
		return "unknown error " + std::to_string(condition);
	}
};

extern ErrorCategory error_category;

std::error_code      make_error_code     (Error e) { return {int(e), error_category}; }
std::error_condition make_error_condition(Error e) { return {int(e), error_category}; }

}

namespace std {

template<> struct is_error_code_enum     <Modbus::Error> : true_type {};
template<> struct is_error_condition_enum<Modbus::Error> : true_type {};

}
