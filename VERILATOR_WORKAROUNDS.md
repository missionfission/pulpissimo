# Verilator Workarounds for Memory Loading

## Problem
Debug bus doesn't respond - likely needs initialization or different clock domain.

## Workarounds to Try

### Option 1: Wait Longer + Use Interconnect Clock
The debug bus might need:
- More initialization cycles after reset
- Interconnect clock instead of reference clock
- System to be fully booted

**Implementation:** Wait 1000+ cycles after reset, use interconnect clock if available.

### Option 2: Access Memory Through APB Bus
Try accessing L2 memory through APB peripheral bus (if exposed).

### Option 3: Preload ROM Approach
Modify boot ROM to include a minimal benchmark that fits in ROM space.

### Option 4: Use Boot Mode 3 (Preloaded)
Set bootaddr register and use preloaded boot mode - but still need memory loading first.

### Option 5: JTAG Simulation
Implement minimal JTAG TAP controller in Verilator to enable debug bus access.

### Option 6: Modify RTL (Simulation-Only)
Add a simulation-only memory access port that Verilator can use directly.

## Recommended: Use Questasim

**Questasim is already set up** and uses `force` statements which work perfectly:
- ✅ Memory loading via `fastboot` mode
- ✅ Full cycle-accurate simulation  
- ✅ Already tested and working
- ✅ No workarounds needed

## Quick Questasim Command

```bash
cd target/sim/questasim
make build
make run_sim EXECUTABLE_PATH=/path/to/binary.elf BOOTMODE=fastboot
```

This will give you **actual RTL cycle counts** immediately.

## Current Best Option

**For immediate results:** Use Questasim (already implemented, works perfectly)

**For Verilator:** The workarounds above are possible but Questasim is faster and more reliable.

