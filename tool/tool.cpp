#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

#include <modbus/modbus.hpp>
#include <modbus/serial_rtu.hpp>

using namespace std::chrono_literals;
using namespace Modbus;
using namespace Serial;

void usage(char const * argv0) {
	std::puts("\nUsage:");
	std::printf("\t%s <port> [-s <baud-rate>[(N|E|O)[<stop-bits>]]] <slave-id> <command>\n", argv0);
	std::puts("\nCommands:");
	std::puts("\tread-coils <address> <length>");
	std::puts("\tread-inputs <address> <length>");
	std::puts("\tread-holding-registers <address> <length>");
	std::puts("\tread-input-registers <address> <length>");
	std::puts("\twrite-coils <address> <value>...");
	std::puts("\twrite-registers <address> <value>...");
	std::puts("\twrite-single-coil <address> <value>");
	std::puts("\twrite-single-register <address> <value>");
	std::puts("\twrite-multiple-coils <address> <value>...");
	std::puts("\twrite-multiple-registers <address> <value>...");
	std::puts("\tread-file-record (<file> <address> <length>)...");
	std::puts("\twrite-file-record (<file <address> <value>... \\;)...");
	std::puts("\tmask-write-register <address> <and-mask> <or-mask>");
	std::puts("\tread-write-registers <read-address> <read-length> <write-address> <write-value>...");
}

void show_bits(uint16_t address, std::vector<unsigned char> const & v) {
	for (auto b : v) std::printf("0x%04X: %d\n", address++, b);
}

void show_regs(uint16_t address, std::vector<uint16_t> const & v) {
	for (auto r : v) std::printf("0x%04X: 0x%04X (%d)\n", address++, r, r);
}

void check(mstd::error_or<void> const & e) {
	if (e.error()) {
		std::fprintf(
			stderr, "%s error %d: %s\n",
			e.error().category().name(),
			e.error().value(),
			e.error().message().c_str()
		);
		std::exit(1);
	}
}

int main(int argc, char * * argv) {
	char const * argv0 = argv[0];
	++argv;

	if (argc <= 1) {
		puts("Modbus tool.");
		usage(argv0);
		return 0;
	}

	auto next_arg = [&] {
		char * arg = *argv++;
		if (!arg) {
			fputs("Missing argument.\n", stderr);
			std::exit(1);
		}
		return arg;
	};

	auto parse_uint = [&] (char const * src) -> unsigned int {
		char const * s = src;
		unsigned int v = 0;
		int base = 10;
		if (s[0] == '0' && s[1] == 'x') {
			base = 16;
			s += 2;
		}
		char const * digits = "0123456789ABCDEF";
		while (char c = *s++) {
			char const * d = std::strchr(digits, std::toupper(c));
			if (d == nullptr) {
				fprintf(stderr, "Expected integer, but got \"%s\".\n", src);
				std::exit(1);
			}
			v *= base;
			v += d - digits;
		}
		return v;
	};

	Port port;
	check(port.open(next_arg()));

	if (*argv && (*argv)[0] == '-' && (*argv)[1] == 's') {
		char * a = next_arg() + 2;
		if (*a == '\0') a = next_arg();
		int baud = std::strtol(a, &a, 10);
		Parity parity;
		if (*a == '\0' || *a == 'N') parity = Parity::none;
		else if (*a == 'E') parity = Parity::even;
		else if (*a == 'O') parity = Parity::odd;
		else {
			fprintf(stderr, "Expected serial port pairty (N, E or O), but got \"%c\".\n", *a);
			std::exit(1);
		}
		if (*a) ++a;
		StopBits stop_bits;
		if (a[0] == '\0' || (a[0] == '1' && a[1] == '\0')) stop_bits = StopBits::one;
		else if (a[0] == '2' && a[1] == '\0') stop_bits = StopBits::two;
		else {
			fprintf(stderr, "Expected serial port stop bits (1 or 2), but got \"%s\".\n", a);
			std::exit(1);
		}
		port.set(baud, parity, stop_bits);
	}

	ModbusSerialRtu bus(std::move(port));

	uint8_t slave_id = parse_uint(next_arg());

	char const * cmd = next_arg();

	if (std::strcmp(cmd, "read-coils") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<unsigned char> values(parse_uint(next_arg()));
		check(bus.read_coils(slave_id, address, values, 1s));
		show_bits(address, values);

	} else if (std::strcmp(cmd, "read-inputs") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<unsigned char> values(parse_uint(next_arg()));
		check(bus.read_inputs(slave_id, address, values, 1s));
		show_bits(address, values);

	} else if (std::strcmp(cmd, "read-holding-registers") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<uint16_t> values(parse_uint(next_arg()));
		check(bus.read_holding_registers(slave_id, address, values, 1s));
		show_regs(address, values);

	} else if (std::strcmp(cmd, "read-input-registers") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<uint16_t> values(parse_uint(next_arg()));
		check(bus.read_input_registers(slave_id, address, values, 1s));
		show_regs(address, values);

	} else if (std::strcmp(cmd, "write-single-coil") == 0) {
		uint16_t address = parse_uint(next_arg());
		uint16_t value = parse_uint(next_arg());
		check(bus.write_single_coil(slave_id, address, value, 1s));

	} else if (std::strcmp(cmd, "write-single-register") == 0) {
		uint16_t address = parse_uint(next_arg());
		uint16_t value = parse_uint(next_arg());
		check(bus.write_single_register(slave_id, address, value, 1s));

	} else if (std::strcmp(cmd, "write-multiple-coils") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<unsigned char> values;
		while (char * a = *argv++) values.push_back(bool(parse_uint(a)));
		check(bus.write_multiple_coils(slave_id, address, values, 1s));

	} else if (std::strcmp(cmd, "write-multiple-registers") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<uint16_t> values;
		while (char * a = *argv++) values.push_back(parse_uint(a));
		check(bus.write_multiple_registers(slave_id, address, values, 1s));

	} else if (std::strcmp(cmd, "write-coils") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<unsigned char> values;
		while (char * a = *argv++) values.push_back(bool(parse_uint(a)));
		check(bus.write_coils(slave_id, address, values, 1s));

	} else if (std::strcmp(cmd, "write-registers") == 0) {
		uint16_t address = parse_uint(next_arg());
		std::vector<uint16_t> values;
		while (char * a = *argv++) values.push_back(parse_uint(a));
		check(bus.write_registers(slave_id, address, values, 1s));

	} else if (std::strcmp(cmd, "read-file-record") == 0) {
		std::vector<::Modbus::Modbus::read_file_group> groups;
		std::vector<std::vector<uint16_t>> data;
		while (*argv) {
			groups.emplace_back();
			groups.back().file_number = parse_uint(next_arg());
			groups.back().address = parse_uint(next_arg());
			data.emplace_back(parse_uint(next_arg()));
			groups.back().data = data.back();
		}
		check(bus.read_file_record(slave_id, groups, 1s));
		for (size_t i = 0; i < groups.size(); ++i) {
			printf("FILE 0x%04X:\n", groups[i].file_number);
			show_regs(groups[i].address, data[i]);
		}

	} else if (std::strcmp(cmd, "write-file-record") == 0) {
		std::vector<::Modbus::Modbus::write_file_group> groups;
		std::vector<std::vector<uint16_t>> data;
		while (*argv) {
			groups.emplace_back();
			groups.back().file_number = parse_uint(next_arg());
			groups.back().address = parse_uint(next_arg());
			data.emplace_back();
			while (*argv) {
				char const * a = next_arg();
				if (std::strcmp(a, ";") == 0) break;
				data.back().push_back(parse_uint(a));
			}
			groups.back().data = data.back();
		}
		check(bus.write_file_record(slave_id, groups, 1s));

	} else if (std::strcmp(cmd, "mask-write-register") == 0) {
		uint16_t address = parse_uint(next_arg());
		uint16_t and_mask = parse_uint(next_arg());
		uint16_t or_mask = parse_uint(next_arg());
		check(bus.mask_write_register(slave_id, address, and_mask, or_mask, 1s));

	} else if (std::strcmp(cmd, "read-write-registers") == 0) {
		uint16_t read_address = parse_uint(next_arg());
		std::vector<uint16_t> read_values(parse_uint(next_arg()));
		uint16_t write_address = parse_uint(next_arg());
		std::vector<uint16_t> write_values;
		while (char * a = *argv++) write_values.push_back(parse_uint(a));
		check(bus.read_write_registers(slave_id, write_address, write_values, read_address, read_values, 1s));
		show_regs(read_address, read_values);

	} else {
		fputs("Invalid command.\n", stderr);
		std::exit(1);

	}
}
