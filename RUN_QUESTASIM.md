# Questasim Execution Instructions

## Prerequisites

1. **Questasim must be installed and in PATH**
   ```bash
   source /path/to/questa/setup.sh  # Or your Questasim setup script
   which vsim  # Should show path to vsim
   ```

## Execution Steps

### Option 1: Automated (Recommended)

```bash
cd sw/regression_tests
./setup_questa.sh
```

This will:
- Check for Questasim
- Compile benchmarks  
- Build Questasim model
- Provide exact commands to run

### Option 2: Manual Step-by-Step

```bash
# Step 1: Setup Questasim environment
source /path/to/questa/setup.sh

# Step 2: Compile benchmarks
cd sw/regression_tests
export PATH="$HOME/.local/riscv-pulp/bin:$PATH"
make motor_control_baseline_O2.elf motor_control_custom_ip_O2.elf

# Step 3: Build Questasim model (takes 5-10 minutes)
cd ../../target/sim/questasim
make build

# Step 4: Run baseline benchmark
cd ../../build/questasim
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_baseline_O2.elf \
    BOOTMODE=fastboot

# Step 5: Run custom IP benchmark
make run_sim \
    EXECUTABLE_PATH=/Users/khushalsethi/pulpissimo/sw/regression_tests/motor_control_custom_ip_O2.elf \
    BOOTMODE=fastboot
```

## Extracting Cycle Counts

Look for this line in the Questasim transcript:
```
[TB] Received status core: 0x8XXXXXXX
```

The cycle count is in the lower 31 bits. For example:
- `0x800000D2` = 210 cycles (bit 31 = done flag)
- Extract: `0x800000D2 & 0x7FFFFFFF = 0xD2 = 210`

## What Happens

1. **Memory Loading**: Binary loaded via `fastboot` mode (uses `force` statements)
2. **Execution**: Code runs on RTL simulation
3. **Completion**: Benchmark writes to 0x1A1040A0 with bit 31 set
4. **Detection**: Testbench reads completion status
5. **Output**: Cycle count printed in transcript

## Files Modified

- `motor_control_baseline.c` - Writes to Questasim completion address
- `motor_control_custom_ip.c` - Writes to Questasim completion address

Both benchmarks now:
- Write cycle count to 0x1A10F000 (for extraction)
- Write completion marker to 0x1A10F004
- Signal completion to testbench at 0x1A1040A0

## Expected Timeline

- **Build**: 5-10 minutes (one time)
- **Baseline simulation**: 1-5 minutes
- **Custom IP simulation**: 1-5 minutes
- **Total**: ~15-20 minutes for first run, ~2-10 minutes for subsequent runs

## Result Format

After running both benchmarks, you'll have:
- Baseline cycles: XXX (from transcript)
- Custom IP cycles: XXX (from transcript)
- Improvement: Calculate percentage
- Speedup: Calculate ratio

Update `CUSTOMER_PERFORMANCE_REPORT.md` with these actual RTL numbers!

