#!/bin/bash
# Setup and Run Motor Control Benchmarks
# This script sets up the environment and runs benchmarks once RISC-V toolchain is available

set -e

echo "=========================================="
echo "Motor Control Benchmark Setup"
echo "=========================================="

# Check for RISC-V toolchain
if command -v riscv32-unknown-elf-gcc &> /dev/null; then
    echo "✓ RISC-V toolchain found: $(which riscv32-unknown-elf-gcc)"
    RISCV_TOOLCHAIN=$(dirname $(dirname $(which riscv32-unknown-elf-gcc)))
    export RISCV=$RISCV_TOOLCHAIN
    export PATH=$RISCV_TOOLCHAIN/bin:$PATH
elif [ -n "$RISCV" ]; then
    echo "✓ Using RISCV environment variable: $RISCV"
    export PATH=$RISCV/bin:$PATH
elif [ -n "$PULP_RISCV_GCC_TOOLCHAIN" ]; then
    echo "✓ Using PULP_RISCV_GCC_TOOLCHAIN: $PULP_RISCV_GCC_TOOLCHAIN"
    export RISCV=$PULP_RISCV_GCC_TOOLCHAIN
    export PATH=$PULP_RISCV_GCC_TOOLCHAIN/bin:$PATH
else
    echo "✗ RISC-V toolchain not found!"
    echo ""
    echo "Please install RISC-V toolchain:"
    echo "  1. Download from: https://github.com/pulp-platform/riscv-gnu-toolchain"
    echo "  2. Or set RISCV environment variable: export RISCV=/path/to/toolchain"
    echo "  3. Or set PULP_RISCV_GCC_TOOLCHAIN: export PULP_RISCV_GCC_TOOLCHAIN=/path/to/toolchain"
    exit 1
fi

# Verify toolchain works
echo ""
echo "Verifying toolchain..."
riscv32-unknown-elf-gcc --version | head -1

# Change to benchmark directory
cd "$(dirname "$0")"
cd sw/regression_tests

echo ""
echo "=========================================="
echo "Building Benchmarks"
echo "=========================================="

# Build all benchmarks with all optimization levels
make clean
make all

echo ""
echo "=========================================="
echo "Build Complete!"
echo "=========================================="
echo ""
echo "Generated binaries:"
ls -lh *.elf 2>/dev/null || echo "No ELF files found"

echo ""
echo "To run benchmarks with Verilator:"
echo "  1. Build RTL: cd ../.. && make build (in target/sim/verilator)"
echo "  2. Run: cd target/sim/verilator && make run_sim EXECUTABLE_PATH=$(pwd)/motor_control_baseline_O2.elf"
echo ""
echo "To analyze code size:"
echo "  riscv32-unknown-elf-size *.elf"

