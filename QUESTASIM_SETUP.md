# Questasim Setup and Run Guide

## Prerequisites

1. **Questasim Installation**
   - Questasim (ModelSim) must be installed
   - Source the setup script: `source /path/to/questa/setup.sh`
   - Or add to PATH: `export PATH=/path/to/questa/bin:$PATH`

2. **Verify Installation**
   ```bash
   which vsim
   vsim -version
   ```

## Running Benchmarks

### Step 1: Build Questasim Model

```bash
cd target/sim/questasim
make build
```

This will:
- Generate compile scripts from Bender
- Compile all RTL files
- Optimize the design

### Step 2: Run Baseline Benchmark

```bash
cd build/questasim
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_baseline_O2.elf \
    BOOTMODE=fastboot
```

### Step 3: Run Custom IP Benchmark

```bash
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_custom_ip_O2.elf \
    BOOTMODE=fastboot
```

### Step 4: Extract Results

The testbench will:
1. Load binary via `fastboot` mode (uses `force` statements)
2. Execute code
3. Wait for completion (reads from 0x1A1040A0)
4. Print exit status

**To extract cycle counts:**
- Check simulation transcript for completion messages
- Read result from memory location 0x1A10F000 (our benchmark writes cycles there)
- Or modify testbench to read from our result location

## Automated Script

Use the provided script:
```bash
cd sw/regression_tests
./run_questa_benchmarks.sh
```

## Notes

- **BOOTMODE=fastboot**: Uses `force` statements for fast memory loading (simulation-only)
- **BOOTMODE=jtag_legacy**: Uses JTAG (slower but more realistic)
- **BOOTMODE=jtag_openocd**: Uses RISC-V debug module (slowest)

For benchmarking, `fastboot` is recommended for speed.

## Troubleshooting

1. **vsim not found**: Source Questasim setup script
2. **Build fails**: Check Bender is installed and Bender.lock exists
3. **Simulation hangs**: Check transcript for errors
4. **No results**: Modify testbench to read from 0x1A10F000 instead of 0x1A1040A0

