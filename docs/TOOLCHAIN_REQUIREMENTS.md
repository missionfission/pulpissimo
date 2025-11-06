# Toolchain Requirements Analysis

## Summary

For compiling and running motor control benchmarks, you need:

### ✅ REQUIRED: RISC-V Toolchain
- **What**: Basic RISC-V GCC compiler (`riscv32-unknown-elf-gcc`)
- **Purpose**: Compile C code to RISC-V binaries
- **Used by**: Bootcode, benchmarks, all bare-metal code
- **Status**: **MUST HAVE** - Cannot compile without it

### ❌ NOT REQUIRED: PULP SDK
- **What**: Full-featured runtime with drivers, tasks, higher-level APIs
- **Purpose**: Provides complete runtime environment, drivers, FreeRTOS support
- **Status**: **OPTIONAL** - Marked as "UNSUPPORTED WITH CURRENT RELEASE" in README
- **When needed**: Only if you need advanced features like FreeRTOS, complex drivers

### ⚠️ OPTIONAL: Simple Runtime (pulp-runtime)
- **What**: Lightweight runtime included as submodule
- **Purpose**: Basic runtime support, simpler than full SDK
- **Status**: **OPTIONAL** - Can compile bare-metal code without it

## For Our Benchmarks

**We only need the RISC-V toolchain** because:

1. **Bootcode compiles without SDK**: The `sw/bootcode/Makefile` uses `riscv32-unknown-elf-gcc` directly
2. **Our benchmarks are bare-metal**: They don't need runtime libraries
3. **We're measuring cycles**: We use CSR cycle counters directly, no runtime needed

## What PULP SDK Provides (if you wanted it)

- **Drivers**: High-level peripheral drivers (UART, SPI, I2C, etc.)
- **Runtime**: Initialization code, memory management
- **FreeRTOS**: Real-time operating system support
- **Build System**: Automated build and simulation integration
- **Examples**: Pre-built example applications

## What We're Using Instead

- **Direct register access**: Using `custom_actuator_ctrl.h` for register access
- **Cycle counters**: Using CSR cycle counters directly (`read_cycle()`)
- **Bare-metal compilation**: Compiling directly with GCC, no runtime needed
- **Custom Makefile**: Our own build system in `sw/regression_tests/Makefile`

## Conclusion

**You only need the RISC-V toolchain** (`riscv32-unknown-elf-gcc`). The PULP SDK is not needed for our benchmarks.

The toolchain can be:
- Downloaded from: https://github.com/pulp-platform/riscv-gnu-toolchain
- Or use a standard RISC-V toolchain with PULP extensions

