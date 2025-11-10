# Verilator Setup for Motor Control Benchmarks

## Overview
This document explains how to use Verilator for RTL simulation of the motor control benchmarks.

## Key Changes for Verilator Compatibility

### 1. No Force Statements
Verilator doesn't support SystemVerilog `force` statements. The testbench has been modified to support conditional compilation:
- When `USE_DEBUG_BUS_DRIVER` is defined: Uses direct signal assignment (requires RTL changes)
- When not defined: Uses original `force` implementation (Questasim only)

### 2. Debug Bus Access
The Verilator testbench (`tb_main.cpp`) accesses the debug bus through Verilator's internal structure using `--public-flat-rw` flag.

### 3. Memory Loading
Memory is loaded through the debug bus after reset, similar to Questasim's `fast_debug_preload` mode but without using `force`.

## Building Verilator Model

```bash
cd target/sim/verilator
make build
```

This will:
1. Parse Bender output to get source files
2. Build Verilator model with `--public-flat-rw` flag
3. Define `USE_DEBUG_BUS_DRIVER` for conditional compilation

## Running Benchmarks

### Option 1: Using Makefile
```bash
cd target/sim/verilator
make run_sim EXECUTABLE_PATH=/path/to/motor_control_baseline_O2.elf
```

### Option 2: Direct Execution
```bash
cd build/verilator
./obj_dir/Vpulpissimo +srec=/path/to/motor_control_baseline_O2.srec +max_cycles=200000
```

## Command Line Arguments

- `+srec=<path>`: Path to SREC file to load
- `+max_cycles=<number>`: Maximum simulation cycles (default: 10000000)
- `+vcd`: Enable VCD tracing (requires Verilator built with --trace)
- `+verbose`: Enable verbose output

## Memory Loading Process

1. **Reset**: System is reset for 10 cycles
2. **Initialization**: Wait 1000 cycles for system initialization
3. **Memory Load**: Load binary through debug bus (if accessible)
4. **Simulation**: Run until benchmark completes or max cycles reached
5. **Result Extraction**: Read cycle count from result memory location (0x1A10F000)

## Debug Bus Access

The Verilator testbench tries to access the debug bus through:
```cpp
top->__PVT__pulpissimo__DOT__i_soc_domain__DOT__i_pulp_soc__DOT__s_lint_debug_bus
```

This requires:
- `--public-flat-rw` flag in Verilator build
- Correct signal path (may vary based on Verilator version)

## Troubleshooting

### Debug Bus Not Accessible
If you see "Warning: Debug bus pointer is null":
1. Check that `--public-flat-rw` flag is in Makefile
2. Verify signal path matches Verilator's generated structure
3. Use `verilator --public` to expose specific signals

### Memory Loading Fails
If memory loading fails:
1. Check that debug bus signals are accessible
2. Increase initialization wait cycles
3. Verify SREC file is valid
4. Check that addresses are in L2 memory range (0x1C000000 - 0x1C090000)

### Build Errors
If Verilator build fails:
1. Check that all stubs are included (FPU, GPIO, etc.)
2. Verify Bender output is correct
3. Check for unsupported SystemVerilog constructs

## Comparison with Questasim

| Feature | Questasim | Verilator |
|---------|-----------|-----------|
| Force statements | ✅ Supported | ❌ Not supported |
| Debug bus access | Via `force` | Via `--public-flat-rw` |
| Boot modes | `fast_debug_preload`, `jtag` | Debug bus only |
| Speed | Slower | Faster |
| License | Commercial | Open source |

## Next Steps

For full Verilator support:
1. ✅ Modified testbench to avoid force statements
2. ✅ Added `--public-flat-rw` flag
3. ⚠️ Verify debug bus access path
4. ⚠️ Test memory loading functionality
5. ⚠️ Extract cycle counts from simulation

