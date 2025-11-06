# Toolchain Setup Guide

## Quick Answer

**You need a RISC-V toolchain, NOT the PULP SDK.**

- **RISC-V Toolchain**: ✅ REQUIRED - Compiler (`riscv32-unknown-elf-gcc`)
- **PULP SDK**: ❌ NOT NEEDED - Optional runtime (marked unsupported in current release)

## What is PULP SDK?

PULP SDK is a **full-featured runtime** that provides:
- High-level drivers (UART, SPI, I2C, etc.)
- FreeRTOS support
- Runtime initialization
- Build system integration
- Example applications

**But it's NOT needed for our benchmarks** because:
1. We use bare-metal code (no runtime)
2. We access registers directly via `custom_actuator_ctrl.h`
3. We use CSR cycle counters directly
4. Bootcode compiles without SDK

## What We Actually Need

### RISC-V Toolchain (REQUIRED)

The RISC-V GCC toolchain with PULP extensions:

```bash
# Download from:
https://github.com/pulp-platform/riscv-gnu-toolchain

# Or use pre-built binaries from releases

# Set environment:
export RISCV=/path/to/riscv-toolchain
export PATH=$RISCV/bin:$PATH

# Or:
export PULP_RISCV_GCC_TOOLCHAIN=/path/to/riscv-toolchain
```

### Verilator (REQUIRED for simulation)

```bash
# Already installed ✓
verilator --version
```

## Current Status

- ✅ Verilator: Installed (version 5.036)
- ❌ RISC-V Toolchain: Not found in PATH

## Next Steps

1. **Install RISC-V Toolchain**:
   ```bash
   # Option 1: Download pre-built
   wget https://github.com/pulp-platform/riscv-gnu-toolchain/releases/...
   
   # Option 2: Build from source
   git clone https://github.com/pulp-platform/riscv-gnu-toolchain
   cd riscv-gnu-toolchain
   ./configure --prefix=/opt/riscv --with-arch=rv32imc
   make
   ```

2. **Set Environment**:
   ```bash
   export RISCV=/opt/riscv
   export PATH=$RISCV/bin:$PATH
   ```

3. **Run Benchmarks**:
   ```bash
   ./run_benchmarks.sh
   ```

## Why Not PULP SDK?

- **Bootcode doesn't use it**: `sw/bootcode/Makefile` compiles directly with GCC
- **Our benchmarks are bare-metal**: No runtime libraries needed
- **SDK is unsupported**: README says "UNSUPPORTED WITH CURRENT RELEASE"
- **We measure cycles directly**: Using CSR cycle counters, no SDK needed

## Summary

| Component | Status | Purpose |
|-----------|--------|---------|
| RISC-V Toolchain | ❌ Need to install | Compile C to RISC-V binaries |
| Verilator | ✅ Installed | RTL simulation |
| PULP SDK | ❌ Not needed | Optional runtime (unsupported) |
| Simple Runtime | ❌ Not needed | Optional lightweight runtime |

**Action**: Install RISC-V toolchain, then run `./run_benchmarks.sh`


