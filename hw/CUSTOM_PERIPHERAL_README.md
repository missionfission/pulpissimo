# Custom Peripheral Integration - Quick Start

## What's Included

This directory contains an example custom peripheral for PULPissimo integration:

**File**: `custom_actuator_ctrl.sv`

A complete memory-mapped peripheral demonstrating:
- Standard APB interface for CPU access
- Control and status registers
- Example actuator control logic with FSM
- Interrupt generation
- External I/O signals

## Key Features

### APB Register Interface
- **Base Address**: 0x1A10_D000
- **Size**: 4KB (0x1000 bytes)
- **6 registers** (see table below)

### Register Map Summary

| Address | Register   | Type | Function                    |
|---------|------------|------|-----------------------------|
| +0x00   | CTRL       | R/W  | Enable, reset, mode control |
| +0x04   | STATUS     | R    | Busy, error, FSM state      |
| +0x08   | POSITION   | R/W  | Target position             |
| +0x0C   | VELOCITY   | R/W  | Movement rate/speed         |
| +0x10   | FEEDBACK   | R    | Sensor feedback input       |
| +0x14   | CONFIG     | R/W  | Interrupt enable, etc.      |

### External Signals

```systemverilog
// Outputs
output logic [15:0] actuator_pwm_o;      // PWM signal
output logic        actuator_dir_o;      // Direction control
output logic        actuator_enable_o;   // Enable output
output logic        interrupt_o;         // Interrupt request

// Inputs  
input  logic [15:0] sensor_feedback_i;   // Position/sensor feedback
```

## How to Use This Template

### For Your Custom IP Block

1. **Copy and rename** `custom_actuator_ctrl.sv` to your module name
2. **Modify the register map** for your application
3. **Replace the FSM/control logic** with your IP's functionality
4. **Update I/O ports** for your specific signals
5. **Update memory map** in `includes/soc_mem_map.svh`

### Software Access

Include the header:
```c
#include "custom_actuator_ctrl.h"
```

Example usage:
```c
custom_actuator_init();
custom_actuator_enable();
custom_actuator_set_position(1000);
while (custom_actuator_is_busy());
```

## Integration Checklist

- [x] RTL module created (`custom_actuator_ctrl.sv`)
- [x] Memory map updated (`soc_mem_map.svh`, `periph_bus_defines.sv`)
- [x] Added to build (`Bender.yml`)
- [x] Software header created (`custom_actuator_ctrl.h`)
- [x] Test program created (`test_custom_actuator.c`)
- [ ] **Connected to APB bus** (requires modifying `pulp_soc` - see guide)
- [ ] Exposed external signals through hierarchy
- [ ] Added to FPGA constraints (if using FPGA)

## Next Steps

See `CUSTOM_IP_INTEGRATION_GUIDE.md` for complete integration instructions, including:
- How to connect to the APB peripheral bus
- Routing signals to top-level/pads
- Building and testing
- Advanced topics (DMA, interrupts, clock domains)

## Customization Examples

### For a Neural Network Accelerator
- Change registers to: CONFIG, COMMAND, DATA_IN, DATA_OUT, STATUS
- Add memory interfaces for weights/activations
- Implement your inference engine logic

### For an ADC Controller  
- Registers: CTRL, CHANNEL_SELECT, SAMPLE_RATE, DATA, STATUS
- Add ADC interface signals (SCLK, MISO, MOSI, CS)
- Implement SPI or other ADC protocol

### For a Signal Processor
- Registers: MODE, INPUT_BUFFER, OUTPUT_BUFFER, COEFF, STATUS
- Add data path for streaming samples
- Implement filter/FFT/correlation logic

## Testing Without Full Integration

Even before connecting to the APB bus, you can:

1. **Standalone testbench**:
   Create `tb_custom_actuator.sv` to verify register operations

2. **Simulation**:
   ```systemverilog
   custom_actuator_ctrl dut (
       .clk_i(clk),
       .rst_ni(rst_n),
       // Drive APB signals from testbench
       .psel_i(psel),
       // ...
   );
   ```

3. **Verify**:
   - Register read/write
   - State machine transitions
   - Output signal generation
   - Interrupt behavior

## Module Template Structure

```systemverilog
module custom_actuator_ctrl (
    // Standard APB interface
    input  clk_i, rst_ni,
    input  psel_i, penable_i, pwrite_i,
    input  [31:0] paddr_i, pwdata_i,
    output [31:0] prdata_o,
    output pready_o, pslverr_o,
    
    // Your custom interface
    output [...] your_outputs,
    input  [...] your_inputs
);

    // 1. Register address decode
    // 2. Write logic for R/W registers
    // 3. Read logic/multiplexer
    // 4. Sequential update of registers
    // 5. YOUR CUSTOM IP LOGIC HERE
    // 6. Output signal generation

endmodule
```

## Resources

- Full integration guide: `../CUSTOM_IP_INTEGRATION_GUIDE.md`
- Software driver: `../sw/bootcode/include/custom_actuator_ctrl.h`
- Test program: `../sw/regression_tests/test_custom_actuator.c`
- Memory map: `includes/soc_mem_map.svh`
