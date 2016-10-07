#pragma once

#include <cstdint>

#include <mstd/range.hpp>

namespace Modbus {

extern std::uint16_t const crc_ibm_table[256];

class crc_ibm {
	std::uint16_t crc_ = 0xFFFF;

public:
	crc_ibm() {}
	crc_ibm(mstd::range<unsigned char const> r) { add(r); }

	crc_ibm & add(mstd::range<unsigned char const> r) {
		for (unsigned char b : r) {
			crc_ = (crc_ >> 8) ^ crc_ibm_table[(crc_ & 0xFF) ^ b];
		}
		return *this;
	}

	std::uint16_t get() const { return crc_; }

	operator std::uint16_t() const { return crc_; }
};

}
