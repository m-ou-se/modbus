# modbus
Cross-platform C++14 modbus library.

# Example

```C++
Port p;
p.open("/dev/ttyUSB0");
p.set(9600, Parity::none, StopBits::two, DataBits::eight);

Modbus::ModbusSerialRtu bus(std::move(p));

std::vector<uint16_t> regs(100);

if (auto e = bus.read_holding_registers(0x12, 0x1000, regs, 100ms).error()) {
	std::cerr << "Error: " << e.message();
	return;
}

for (uint16_t r : regs) {
	std::cout << r << std::endl;
}
```

## Documentation

See [modbus.hpp](include/modbus/modbus.hpp).

## Dependencies

- [serial](https://github.com/m-ou-se/serial)
- [mstd](https://github.com/m-ou-se/mstd)

## License

Two-clause BSD license, see [COPYING](COPYING).
