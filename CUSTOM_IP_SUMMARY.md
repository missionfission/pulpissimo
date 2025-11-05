# Custom IP Integration Summary

## Overview

A complete example has been created showing how to integrate custom IP blocks into PULPissimo as memory-mapped peripherals for silicon and software validation.

## What Was Created

### 1. RTL Peripheral Module
**File**: `hw/custom_actuator_ctrl.sv` (9.2 KB)
- Complete APB-interfaced peripheral
- 6 memory-mapped registers (control, status, position, velocity, feedback, config)
- Example actuator control FSM
- PWM generation logic
- Interrupt support
- External I/O signals

**Memory Mapped At**: `0x1A10_D000 - 0x1A10_DFFF`

### 2. Software Driver Header
**File**: `sw/bootcode/include/custom_actuator_ctrl.h` (7.1 KB)
- Register address definitions
- Bit field definitions
- Helper functions for all operations:
  - `custom_actuator_init()`
  - `custom_actuator_enable()` / `custom_actuator_disable()`
  - `custom_actuator_set_position()`
  - `custom_actuator_set_velocity()`
  - `custom_actuator_read_feedback()`
  - Status checking functions
  - Interrupt control

### 3. Test Program
**File**: `sw/regression_tests/test_custom_actuator.c` (5.5 KB)
- Comprehensive test suite with 10 test cases:
  1. Initialization
  2. Enable/disable
  3. Position control
  4. Mode switching
  5. Register read/write verification
  6. Feedback sensor reading
  7. Software reset
  8. Interrupt configuration
  9. Status monitoring
  10. Cleanup

### 4. Documentation
- **`CUSTOM_IP_INTEGRATION_GUIDE.md`**: Complete integration guide (15+ KB)
  - Step-by-step integration instructions
  - Two integration approaches (fork pulp_soc or use chip_ctrl port)
  - Building and testing instructions
  - Customization guide
  - Debugging tips
  - Advanced topics

- **`hw/CUSTOM_PERIPHERAL_README.md`**: Quick reference guide
  - Register map summary
  - Usage examples
  - Integration checklist
  - Customization examples for different IP types

### 5. Modified System Files

**`hw/includes/soc_mem_map.svh`**:
- Added memory map definitions for custom peripheral

**`hw/includes/periph_bus_defines.sv`**:
- Incremented `NB_MASTER` from 11 to 12
- Added address range defines

**`Bender.yml`**:
- Added `hw/custom_actuator_ctrl.sv` to build sources

## Quick Start

### 1. Review the Example Peripheral

```bash
# Look at the RTL implementation
cat hw/custom_actuator_ctrl.sv

# Check the software interface
cat sw/bootcode/include/custom_actuator_ctrl.h

# Review the test program
cat sw/regression_tests/test_custom_actuator.c
```

### 2. Understand the Integration

Read the integration guide:
```bash
cat CUSTOM_IP_INTEGRATION_GUIDE.md
```

### 3. Customize for Your IP

Use the example as a template:
1. Copy `custom_actuator_ctrl.sv` and rename
2. Modify registers for your application
3. Replace FSM with your IP logic
4. Update software header
5. Write tests

## Register Map

| Offset | Name         | Access | Description                    |
|--------|--------------|--------|--------------------------------|
| 0x00   | CTRL_REG     | R/W    | Enable, reset, mode (4 bits)   |
| 0x04   | STATUS_REG   | R      | Busy, error, FSM state         |
| 0x08   | POSITION_REG | R/W    | Target position/value          |
| 0x0C   | VELOCITY_REG | R/W    | Rate/speed control             |
| 0x10   | FEEDBACK_REG | R      | Sensor/feedback input (16-bit) |
| 0x14   | CONFIG_REG   | R/W    | Interrupt enable, etc.         |

## Example Software Usage

```c
#include "custom_actuator_ctrl.h"

int main() {
    // Initialize peripheral
    custom_actuator_init();
    custom_actuator_enable();
    
    // Configure operation
    custom_actuator_set_mode(CUSTOM_ACTUATOR_MODE_POSITION);
    custom_actuator_set_position(1000);
    custom_actuator_set_velocity(5000);
    
    // Monitor progress
    while (custom_actuator_is_busy()) {
        uint16_t pos = custom_actuator_read_feedback();
        printf("Position: %d\n", pos);
    }
    
    // Check completion
    if (!custom_actuator_has_error()) {
        printf("Success!\n");
    }
    
    return 0;
}
```

## Integration Status

✅ **Complete**:
- RTL peripheral module with APB interface
- Memory map allocation
- Software driver header with helper functions
- Comprehensive test program
- Documentation and guides

⚠️ **Requires User Action**:
- **Connect to APB bus**: Must modify `pulp_soc` module or use alternative integration approach (see guide)
- **Route external signals**: Must connect actuator I/O through module hierarchy to pads
- **Build system**: May need to update synthesis/simulation scripts depending on toolchain

## Use Cases

This template supports integration of:

### Motor/Actuator Control
- PWM generation
- Encoder feedback processing
- Motion control algorithms
- Stepper motor control

### Signal Processing
- Digital filters (FIR, IIR)
- FFT/IFFT accelerators
- Correlation engines
- Feature extraction

### Neural Networks
- Small inference accelerators
- Quantized network engines
- Activation functions
- MAC units

### Custom Peripherals
- ADC/DAC controllers
- Sensor interfaces (SPI, I2C extensions)
- Custom communication protocols
- Timing/measurement units

### Security/Crypto
- AES/SHA accelerators
- Random number generators
- Key storage
- Secure boot helpers

## Next Steps

### 1. For Testing This Example
1. Complete the APB bus integration (see guide section "Integration Steps")
2. Build simulation environment
3. Compile and run test program
4. Verify all register operations

### 2. For Your Custom IP
1. Use `custom_actuator_ctrl.sv` as a template
2. Define your register map
3. Implement your IP's logic
4. Create corresponding software driver
5. Write comprehensive tests

### 3. For FPGA Deployment
1. Add pin constraints for external signals
2. Update padframe configuration
3. Build FPGA bitstream
4. Test on hardware

## File Locations Summary

```
PULPissimo/
├── hw/
│   ├── custom_actuator_ctrl.sv              # ← RTL peripheral
│   ├── CUSTOM_PERIPHERAL_README.md          # ← Quick reference
│   └── includes/
│       ├── soc_mem_map.svh                  # ← Modified (memory map)
│       └── periph_bus_defines.sv            # ← Modified (APB bus)
├── sw/
│   ├── bootcode/include/
│   │   └── custom_actuator_ctrl.h           # ← Software driver
│   └── regression_tests/
│       └── test_custom_actuator.c           # ← Test program
├── Bender.yml                               # ← Modified (build system)
├── CUSTOM_IP_INTEGRATION_GUIDE.md           # ← Full guide
└── CUSTOM_IP_SUMMARY.md                     # ← This file
```

## Key Characteristics

### RTL Peripheral
- **Interface**: Standard AMBA APB
- **Clock**: `per_clk` (peripheral clock domain)
- **Reset**: Active-low `rst_ni`
- **Latency**: Single-cycle register access (pready always 1)
- **Address**: 32-bit byte-addressed
- **Data**: 32-bit word

### Software Access
- **Volatile register accesses**: Ensures no optimization
- **Inline functions**: Zero overhead
- **Type-safe**: Uses stdint.h types
- **Portable**: Standard C99

## Testing Checklist

Before deploying your custom IP:

- [ ] All registers read/write correctly
- [ ] Reset behavior verified
- [ ] FSM state transitions correct
- [ ] External outputs generate expected signals
- [ ] Interrupts fire at appropriate times
- [ ] Error conditions handled
- [ ] Performance meets requirements
- [ ] No timing violations in synthesis
- [ ] Software driver matches hardware behavior
- [ ] Documentation updated

## Support and References

- **PULPissimo Repository**: https://github.com/pulp-platform/pulpissimo
- **PULP SoC Repository**: https://github.com/pulp-platform/pulp_soc
- **PULP SDK**: https://github.com/pulp-platform/pulp-sdk
- **APB Specification**: ARM AMBA APB Protocol Specification

## Contributing

To contribute improvements to this example:
1. Test your changes in simulation
2. Verify on FPGA if possible
3. Update documentation
4. Submit pull request with clear description

---

**Last Updated**: 2025-11-04  
**Version**: 1.0  
**Status**: Ready for integration testing
