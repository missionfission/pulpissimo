# Intercepting load_l2 Without Force Statements

## Problem
The `preload_l2` task uses SystemVerilog `force` statements to directly drive internal `tcdm_debug` bus signals. This doesn't work in Verilator, which doesn't support `force`.

## Current Implementation
- **`preload_l2`** (line 842): Uses `force` to drive `tcdm_debug` signals
- **Boot mode**: `fast_debug_preload` uses this task
- **Issue**: Verilator doesn't support `force` statements

## Solutions

### Option 1: Use JTAG Boot Modes (No RTL Changes Required)
The testbench already supports JTAG-based loading that doesn't use `force`:

```systemverilog
// In tb_pulp.sv line 764-776
if (bootmode == "jtag") begin
  // Uses pulp_tap_pkg::load_L2 or debug_mode_if.load_L2
  // Both use JTAG TAP interface - no force needed
end
```

**Usage:**
```bash
# Use JTAG boot mode instead of fast_debug_preload
make run_sim BOOTMODE=jtag EXECUTABLE_PATH=...
```

**Pros:**
- No RTL changes needed
- Works in both Questasim and Verilator
- Already implemented and tested

**Cons:**
- Slower than fastboot (uses JTAG protocol)
- More realistic (matches hardware behavior)

### Option 2: Expose Debug Bus as Ports (Requires RTL Changes)
Modify the RTL to expose `tcdm_debug` signals as ports:

**In `hw/soc_domain.sv` or interconnect wrapper:**
```systemverilog
// Add debug bus port
output XBAR_TCDM_BUS debug_bus_o,
input  XBAR_TCDM_BUS debug_bus_i  // For reading responses
```

**In testbench:**
```systemverilog
// Connect directly without force
assign i_dut.debug_bus_o.req = debug_bus_req;
assign i_dut.debug_bus_o.add = debug_bus_add;
// ... etc
```

**Pros:**
- Fast boot mode works
- No force statements needed
- Verilator-compatible

**Cons:**
- Requires RTL modifications
- Changes design interface
- May affect synthesis/FPGA builds

### Option 3: Use DPI-C Interface (Hybrid Approach)
Create a DPI-C interface that can be called from C++ (Verilator) or SystemVerilog:

**Created:** `target/sim/tb/tb_lib/debug_bus_dpi.sv`

**Pros:**
- Can work with both simulators
- No force statements

**Cons:**
- Requires implementing C++ side for Verilator
- More complex integration

### Option 4: Modified preload_l2 with Conditional Compilation
The modified `preload_l2` task now supports both modes:

```systemverilog
`ifdef USE_DEBUG_BUS_DRIVER
  // Direct assignment (requires RTL changes to expose signals)
`else
  // Original force-based implementation
`endif
```

**To use:**
1. Expose debug bus signals in RTL (Option 2)
2. Define `USE_DEBUG_BUS_DRIVER` when compiling
3. Use `bootmode=fast_debug_preload`

## Recommendation

**For Verilator:** Use `BOOTMODE=jtag` - it's already implemented and doesn't use force.

**For Questasim:** Continue using `BOOTMODE=fast_debug_preload` - it works fine with force.

**For both:** If you need fast boot in Verilator, implement Option 2 (expose debug bus ports).

## Current Status

The testbench has been modified to support conditional compilation:
- When `USE_DEBUG_BUS_DRIVER` is defined: Uses direct assignment (requires RTL changes)
- When not defined: Uses original `force` implementation (Questasim only)

The debug bus signals are declared but need to be connected to actual RTL ports for the non-force path to work.

