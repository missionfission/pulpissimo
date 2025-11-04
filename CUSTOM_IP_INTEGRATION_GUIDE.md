# Custom IP Block Integration Guide for PULPissimo

This guide demonstrates how to integrate a custom IP block (e.g., actuator controller, neural engine, signal processor) into PULPissimo as a memory-mapped peripheral for silicon and software validation.

## Overview

This example includes:
- **Custom RTL Peripheral**: `hw/custom_actuator_ctrl.sv` - Example actuator controller with APB interface
- **Memory Map**: Updated to allocate address space 0x1A10_D000 - 0x1A10_DFFF
- **Software Driver**: `sw/bootcode/include/custom_actuator_ctrl.h` - C header with register definitions
- **Test Program**: `sw/regression_tests/test_custom_actuator.c` - Comprehensive test suite

## Files Created/Modified

### New Files
```
hw/custom_actuator_ctrl.sv                      # Custom peripheral RTL
sw/bootcode/include/custom_actuator_ctrl.h      # Software driver header
sw/regression_tests/test_custom_actuator.c      # Test program
```

### Modified Files
```
hw/includes/soc_mem_map.svh                     # Added memory map entry
hw/includes/periph_bus_defines.sv               # Added APB bus entry (NB_MASTER: 11→12)
Bender.yml                                      # Added custom_actuator_ctrl.sv to sources
```

## Integration Steps

### Step 1: Connect to APB Peripheral Bus

The custom peripheral needs to be connected to the APB peripheral interconnect inside `pulp_soc`. Since `pulp_soc` is an external dependency, you have two options:

#### Option A: Fork and Modify pulp_soc (Recommended for permanent integration)

1. **Fork the pulp_soc repository**:
   ```bash
   git clone https://github.com/pulp-platform/pulp_soc.git
   cd pulp_soc
   ```

2. **Locate the APB interconnect** (typically in `rtl/pulp_soc/soc_peripherals.sv` or similar):
   ```systemverilog
   apb_node_wrap #(
       .NB_MASTER     ( `NB_MASTER ),  // Now 12 instead of 11
       // ...
   ) i_apb_node (
       // existing connections...
   );
   ```

3. **Add your peripheral instance**:
   ```systemverilog
   custom_actuator_ctrl i_custom_actuator (
       .clk_i      ( clk_i         ),
       .rst_ni     ( rst_ni        ),
       .psel_i     ( apb_custom_actuator_slave.psel    ),
       .penable_i  ( apb_custom_actuator_slave.penable ),
       .pwrite_i   ( apb_custom_actuator_slave.pwrite  ),
       .paddr_i    ( apb_custom_actuator_slave.paddr   ),
       .pwdata_i   ( apb_custom_actuator_slave.pwdata  ),
       .prdata_o   ( apb_custom_actuator_slave.prdata  ),
       .pready_o   ( apb_custom_actuator_slave.pready  ),
       .pslverr_o  ( apb_custom_actuator_slave.pslverr ),
       
       // Custom IP signals - connect to top-level ports
       .actuator_pwm_o     ( actuator_pwm_o     ),
       .actuator_dir_o     ( actuator_dir_o     ),
       .actuator_enable_o  ( actuator_enable_o  ),
       .sensor_feedback_i  ( sensor_feedback_i  ),
       .interrupt_o        ( custom_actuator_irq )
   );
   ```

4. **Update the APB node to include your peripheral**:
   Add the custom actuator to the slave list in the APB interconnect configuration.

5. **Update your Bender.yml** to use your forked pulp_soc:
   ```yaml
   dependencies:
     pulp_soc: { git: "https://github.com/YOUR_USERNAME/pulp_soc.git", version: "your-branch" }
   ```

#### Option B: Use apb_chip_ctrl_master Port (For external platform-specific peripherals)

The `soc_domain` module already exposes `apb_chip_ctrl_master` port for platform-specific peripherals:

1. **Create a wrapper module** in `hw/custom_peripherals_wrapper.sv`:
   ```systemverilog
   module custom_peripherals_wrapper (
       input  logic         clk_i,
       input  logic         rst_ni,
       APB.Slave            apb_slave,
       // Custom IP external signals
       output logic [15:0]  actuator_pwm_o,
       output logic         actuator_dir_o,
       output logic         actuator_enable_o,
       input  logic [15:0]  sensor_feedback_i,
       output logic         interrupt_o
   );
   
       // APB address decoder for multiple custom peripherals if needed
       // For single peripheral, pass through directly
       
       custom_actuator_ctrl i_custom_actuator (
           .clk_i,
           .rst_ni,
           .psel_i    ( apb_slave.psel    ),
           .penable_i ( apb_slave.penable ),
           .pwrite_i  ( apb_slave.pwrite  ),
           .paddr_i   ( apb_slave.paddr   ),
           .pwdata_i  ( apb_slave.pwdata  ),
           .prdata_o  ( apb_slave.prdata  ),
           .pready_o  ( apb_slave.pready  ),
           .pslverr_o ( apb_slave.pslverr ),
           .actuator_pwm_o,
           .actuator_dir_o,
           .actuator_enable_o,
           .sensor_feedback_i,
           .interrupt_o
       );
   
   endmodule
   ```

2. **Modify `soc_domain.sv`** or `pulpissimo.sv` to instantiate this wrapper and connect to the chip_ctrl port.

### Step 2: Export Custom Signals to Top Level

Add your custom IP's external signals to the module hierarchy:

1. **In `soc_domain.sv`**, add ports:
   ```systemverilog
   module soc_domain #(
       // existing parameters...
   )(
       // existing ports...
       
       // Custom Actuator Controller
       output logic [15:0]  actuator_pwm_o,
       output logic         actuator_dir_o,
       output logic         actuator_enable_o,
       input  logic [15:0]  sensor_feedback_i
   );
   ```

2. **In `pulpissimo.sv`**, add corresponding ports and connect through the hierarchy.

3. **In FPGA/ASIC top level**, connect these to pads via the padframe.

## RTL Structure

### Custom Peripheral Module Interface

```systemverilog
module custom_actuator_ctrl #(
    parameter int unsigned ADDR_WIDTH = 32,
    parameter int unsigned DATA_WIDTH = 32
)(
    // Clock and Reset
    input  logic                  clk_i,
    input  logic                  rst_ni,
    
    // APB Interface (Standard AMBA APB)
    input  logic                  psel_i,
    input  logic                  penable_i,
    input  logic                  pwrite_i,
    input  logic [ADDR_WIDTH-1:0] paddr_i,
    input  logic [DATA_WIDTH-1:0] pwdata_i,
    output logic [DATA_WIDTH-1:0] prdata_o,
    output logic                  pready_o,
    output logic                  pslverr_o,
    
    // Custom IP Interface - Your specific signals
    output logic [15:0]           actuator_pwm_o,
    output logic                  actuator_dir_o,
    output logic                  actuator_enable_o,
    input  logic [15:0]           sensor_feedback_i,
    output logic                  interrupt_o
);
```

### Register Map

| Offset | Name          | Access | Description                           |
|--------|---------------|--------|---------------------------------------|
| 0x00   | CTRL_REG      | R/W    | Control register (enable, mode, etc.) |
| 0x04   | STATUS_REG    | R      | Status (busy, error, FSM state)       |
| 0x08   | POSITION_REG  | R/W    | Target position/value                 |
| 0x0C   | VELOCITY_REG  | R/W    | Velocity/rate control                 |
| 0x10   | FEEDBACK_REG  | R      | Sensor feedback value                 |
| 0x14   | CONFIG_REG    | R/W    | Configuration (interrupts, etc.)      |

## Software Integration

### Include the Driver Header

```c
#include "custom_actuator_ctrl.h"
```

### Basic Usage Example

```c
// Initialize the peripheral
custom_actuator_init();

// Enable the actuator
custom_actuator_enable();

// Set operating mode
custom_actuator_set_mode(CUSTOM_ACTUATOR_MODE_POSITION);

// Set target position and velocity
custom_actuator_set_position(1000);
custom_actuator_set_velocity(5000);

// Wait for operation to complete
while (custom_actuator_is_busy()) {
    uint16_t feedback = custom_actuator_read_feedback();
    printf("Current position: %d\n", feedback);
}

// Check for errors
if (custom_actuator_has_error()) {
    printf("Error occurred!\n");
}

// Disable when done
custom_actuator_disable();
```

## Building and Testing

### Compile the Test Program

```bash
cd sw/regression_tests
# Use your PULP SDK toolchain
riscv32-unknown-elf-gcc -o test_custom_actuator.elf test_custom_actuator.c \
    -I../bootcode/include -march=rv32imc -O2
```

### Run in Simulation

```bash
cd target/sim/questasim
make clean all
# Load and run test_custom_actuator.elf
```

### Expected Test Output

```
==============================================
Custom Actuator Controller Test
==============================================

Test 1: Initialization
Status: 0x00000000 | State: 0x00 | Busy: 0 | Error: 0 | Feedback: 0

Test 2: Enable Actuator
Status: 0x00000101 | State: 0x01 | Busy: 1 | Error: 0 | Feedback: 0

Test 3: Position Control
Target Position: 1000
Velocity: 5000
  t=0: Status: 0x00000201 | State: 0x02 | Busy: 1 | Error: 0 | Feedback: 0
  ...
  Position reached!
...
```

## Customizing for Your IP Block

To adapt this example for your specific IP:

### 1. Modify the RTL (`hw/custom_actuator_ctrl.sv`)

- **Change the register map** to match your IP's needs
- **Replace the example FSM** with your actual control logic
- **Update the I/O ports** for your specific application
- **Add your custom logic** in the "Custom IP Logic" section

### 2. Update the Software Driver (`sw/bootcode/include/custom_actuator_ctrl.h`)

- **Update register offsets and bit definitions**
- **Modify helper functions** for your IP's operations
- **Add any IP-specific configuration functions**

### 3. Update Memory Map

- Choose an available address range in `soc_mem_map.svh`
- Ensure no conflicts with existing peripherals
- Update base address in the header file

### 4. Create Appropriate Tests

- Modify `test_custom_actuator.c` for your IP's functionality
- Test all registers and modes
- Verify timing and performance

## Advanced Integration Topics

### Adding DMA Support

If your IP needs DMA:
1. Connect to the UDMA subsystem
2. Implement UDMA channel interfaces
3. Configure DMA descriptors in software

### Interrupt Handling

To handle interrupts from your IP:
1. Connect `interrupt_o` to the interrupt controller
2. Register an interrupt handler in software
3. Implement ISR to clear interrupt flags

### Clock Domain Crossing

If your IP runs at a different clock:
1. Add clock domain crossing (CDC) logic
2. Use proper synchronizers for control signals
3. Follow PULPissimo clocking guidelines

### FPGA Constraints

For FPGA deployment:
1. Add timing constraints for your IP's paths
2. Update pin assignments in `target/fpga/pulpissimo-*/constraints/`
3. Modify padframe configuration if adding external pins

## Debugging Tips

### Common Issues

1. **Peripheral not responding**:
   - Check address decoding in APB interconnect
   - Verify clock and reset connections
   - Check pready/psel handshake

2. **Wrong data read back**:
   - Verify register address calculation (byte vs word aligned)
   - Check endianness
   - Verify APB timing (penable, psel sequence)

3. **Simulation failures**:
   - Check for uninitialized signals
   - Verify reset behavior
   - Look for combinational loops

### Useful Debug Techniques

```systemverilog
// Add assertions for protocol checking
assert property (@(posedge clk_i) psel_i |-> ##1 penable_i)
    else $error("APB protocol violation: penable without psel");

// Add debug registers
logic [31:0] debug_counter;
always_ff @(posedge clk_i) begin
    if (apb_write) debug_counter <= debug_counter + 1;
end
```

## References

- [AMBA APB Protocol Specification](https://developer.arm.com/documentation/ihi0024/latest/)
- [PULPissimo Documentation](https://github.com/pulp-platform/pulpissimo)
- [PULP SDK](https://github.com/pulp-platform/pulp-sdk)

## Example Use Cases

This integration pattern works for:
- **Motor/Actuator Controllers**: PWM generation, encoder feedback
- **Signal Processing Accelerators**: FFT, filters, correlation
- **Neural Network Engines**: Small inference accelerators
- **Custom Peripherals**: ADC/DAC control, sensor interfaces
- **Communication Controllers**: Custom protocols, bus interfaces
- **Security Accelerators**: Crypto engines, random number generators

## License

This example code is provided under the Solderpad Hardware License v0.51 (SHL-0.51).
