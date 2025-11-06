# Toolchain Installation Complete! ✅

## What Was Installed

1. **RISC-V Toolchain** (via Homebrew)
   - Location: `/opt/homebrew/bin/riscv64-elf-gcc`
   - Symlinked to: `~/.local/riscv-pulp/bin/riscv32-unknown-elf-gcc`
   - Version: GCC 15.2.0

2. **Environment Setup**
   - Added to `~/.zshrc`:
     - `RISCV=$HOME/.local/riscv-pulp`
     - `PATH=$RISCV/bin:$PATH`
     - `PULP_RISCV_GCC_TOOLCHAIN=$RISCV`

## Benchmarks Built Successfully

All benchmark binaries have been compiled:

### Baseline (Software-only)
- `motor_control_baseline_O2.elf` - O2 optimization
- `motor_control_baseline_O3.elf` - O3 optimization  
- `motor_control_baseline_Ofast.elf` - Ofast optimization

### Custom IP (Hardware-accelerated)
- `motor_control_custom_ip_O2.elf` - O2 optimization
- `motor_control_custom_ip_O3.elf` - O3 optimization
- `motor_control_custom_ip_Ofast.elf` - Ofast optimization

## Next Steps: Run Benchmarks

### Option 1: Using Verilator (when RTL is built)
```bash
cd target/sim/verilator
make build
make run_sim EXECUTABLE_PATH=/path/to/motor_control_baseline_O2.elf
```

### Option 2: Using Questasim (if available)
```bash
cd target/sim/questasim
make build
make run_sim EXECUTABLE_PATH=/path/to/motor_control_baseline_O2.elf
```

### Option 3: Analyze Binaries
```bash
cd sw/regression_tests
riscv32-unknown-elf-objdump -d motor_control_baseline_O2.elf > baseline_O2.objdump
riscv32-unknown-elf-objdump -d motor_control_custom_ip_O2.elf > custom_ip_O2.objdump
```

## Verification

To verify the toolchain is working:
```bash
source ~/.zshrc  # or start a new terminal
riscv32-unknown-elf-gcc --version
```

## Files Created

- `setup_toolchain.sh` - Toolchain setup script
- `run_benchmarks.sh` - Benchmark runner script
- All benchmark ELF binaries in `sw/regression_tests/`

## Note

The toolchain installed is `riscv64-elf-gcc` (64-bit) symlinked as `riscv32-unknown-elf-gcc`. 
For full PULP extensions support, you may need to build the PULP-specific toolchain from source,
but the current toolchain should work for basic benchmarks.


