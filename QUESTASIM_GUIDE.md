# Questasim Setup and Execution Guide

## Status

✅ **Benchmarks Updated** - Now write to Questasim completion address (0x1A1040A0)
✅ **Testbench Ready** - Uses `tb_pulp` with fastboot mode
✅ **Build System Ready** - Makefile configured

## Setup Questasim

### Step 1: Locate Questasim Installation

Questasim is typically installed in one of these locations:
- `/opt/mentor/questa/.../bin`
- `/tools/mentor/questa/.../bin`  
- `/usr/local/questa/.../bin`
- Or check your license server location

### Step 2: Source Setup Script

```bash
# Find and source the setup script
source /path/to/questa/setup.sh

# Or add to PATH manually
export PATH=/path/to/questa/bin:$PATH
```

### Step 3: Verify Installation

```bash
which vsim
vsim -version
```

## Running Benchmarks

### Quick Start

```bash
# 1. Setup Questasim (source setup script first)
source /path/to/questa/setup.sh

# 2. Run setup script
cd sw/regression_tests
./setup_questa.sh

# 3. Follow the instructions printed by setup_questa.sh
```

### Manual Execution

```bash
# 1. Build Questasim model (one time, takes ~5-10 minutes)
cd target/sim/questasim
make build

# 2. Run baseline benchmark
cd build/questasim
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_baseline_O2.elf \
    BOOTMODE=fastboot

# 3. Run custom IP benchmark
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_custom_ip_O2.elf \
    BOOTMODE=fastboot
```

## Extracting Results

The testbench will output:
```
[TB] Received status core: 0x8XXXXXXX
```

Where:
- **Bit 31 = 1**: Benchmark completed
- **Bits [30:0]**: Cycle count (if stored there)

**To extract cycle count:**
```bash
# From transcript, extract the hex value
# Example: 0x800000D2 = 210 cycles
#          (bit 31 = done, lower bits = cycle count)
```

## Benchmark Output Format

Our benchmarks write:
- **0x1A10F000**: Cycle count (full 32-bit value)
- **0x1A10F004**: Completion marker (0xDEADBEEF)
- **0x1A1040A0**: Questasim exit status (bit 31 = done, bits [30:0] = cycles)

The testbench reads from **0x1A1040A0** to detect completion.

## Expected Output

```
[TB] Waiting for end of computation
[TB] Received status core: 0x800000D2  (210 cycles, done)
[TB] TB Execution finished with exit code 0
```

## Troubleshooting

1. **vsim not found**: Source Questasim setup script
2. **Build fails**: Check Bender is installed: `which bender`
3. **Simulation hangs**: Check transcript for errors
4. **No completion**: Verify benchmark writes to 0x1A1040A0

## Next Steps After Getting Results

Once you have cycle counts from Questasim:
1. Update `CUSTOMER_PERFORMANCE_REPORT.md` with actual RTL numbers
2. Calculate improvement percentage
3. Generate final customer presentation

