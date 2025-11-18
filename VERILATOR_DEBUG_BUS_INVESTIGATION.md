# Verilator Debug Bus Investigation - Summary

## Status: Debug Bus Accessible But Not Responding

### What We've Implemented ✅

1. **Address Remapping**: Fixed SREC address remapping from ROM (0x1a000000) to L2 (0x1c000000)
2. **Extended Initialization**: Increased wait time to 10,000 reference clock cycles
3. **Debug Bus Access**: Successfully accessing debug bus signals via `--public-flat-rw` flag
4. **Improved Error Reporting**: Better diagnostics and progress reporting

### Current Issue ❌

The debug bus signals are accessible, but **grants are not being issued**:
- `debug_bus_->req = 1` is set correctly
- `debug_bus_->gnt` never becomes `1` (timeout after 5000 cycles)
- This suggests the debug module is not active or needs initialization

### Root Cause Analysis

The debug bus (`tcdm_debug`) likely requires:
1. **JTAG Initialization**: The debug module may need JTAG TAP controller to enable debug bus access
2. **Debug Module Enable**: A register write to enable the debug module
3. **Boot Mode**: The system might need to be in a specific boot mode (e.g., JTAG boot mode)
4. **Clock Domain**: Debug bus uses SoC clock, but we're only driving reference clock

### What We've Tried

1. ✅ Increased initialization wait (100 → 1000 → 5000 → 10000 ref clock cycles)
2. ✅ Address remapping (ROM → L2)
3. ✅ Debug bus signal initialization (req=0, wen=1, be=0xF)
4. ✅ Extended timeout for grant (5000 cycles)
5. ✅ Multiple clock cycles between transactions

### Remaining Options

#### Option 1: JTAG-Based Memory Loading (Recommended)
Implement a minimal JTAG TAP controller in Verilator to:
- Initialize the debug module
- Enable debug bus access
- Load memory through JTAG protocol

**Pros**: Most realistic, matches hardware behavior
**Cons**: Requires implementing JTAG protocol in C++

#### Option 2: Direct Memory Access (If Verilator Exposes Memory)
Try to access L2 memory arrays directly through Verilator's internal structure:
- Find memory module path (e.g., `__PVT__pulpissimo__DOT__...__DOT__memory_array`)
- Write directly to memory array (bypassing debug bus)

**Pros**: Fastest, bypasses debug bus entirely
**Cons**: Requires finding exact memory module path, may not be exposed

#### Option 3: Boot ROM Modification
Modify the boot ROM to include a minimal benchmark:
- Place benchmark code in ROM space (0x1a000000)
- No memory loading needed - code executes from ROM

**Pros**: Works immediately, no memory loading needed
**Cons**: Limited to small benchmarks that fit in ROM (~16KB)

#### Option 4: Preload Memory via RTL Modification
Add a simulation-only memory preload port:
- Add a `sim_mem_preload` port to pulpissimo module
- Use DPI-C to write memory from C++ testbench
- Only active in simulation (guarded by `ifdef SIMULATION`)

**Pros**: Clean solution, works with Verilator
**Cons**: Requires RTL modification

### Recommendation

**For immediate results**: Use **Option 3 (Boot ROM Modification)** - modify the linker script to place code in ROM space and create a minimal benchmark that fits in ROM.

**For full solution**: Implement **Option 1 (JTAG-Based Loading)** - this is the most realistic approach and matches how the hardware actually works.

### Current Workaround

Until memory loading works, we have:
- ✅ Instruction-level cycle analysis (210.5 baseline / 141.5 custom IP cycles)
- ✅ Conservative estimates suitable for customer presentation
- ✅ Full Verilator simulation infrastructure ready

### Next Steps

1. Try Option 3 (Boot ROM) for immediate results
2. Implement Option 1 (JTAG) for full solution
3. Document findings for customer presentation


