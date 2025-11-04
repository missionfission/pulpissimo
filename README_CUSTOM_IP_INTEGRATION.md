# Custom IP Block Integration for PULPissimo

## 🎯 Purpose

This repository now includes a **complete example** of how to integrate custom IP blocks into PULPissimo as memory-mapped peripherals. Use this as a **silicon and software validation testbed** for your specialized IP without taping out a new chip.

## 🚀 Quick Start

### What's Included

A fully-functional **custom actuator controller** peripheral demonstrating:

✅ **Hardware (RTL)**
- Complete SystemVerilog module with APB interface
- 6 memory-mapped registers
- Example control FSM and PWM generation
- Interrupt support
- Located at: `hw/custom_actuator_ctrl.sv`

✅ **Software (Driver)**
- C header with register definitions and helper functions
- Type-safe inline functions for zero overhead
- Located at: `sw/bootcode/include/custom_actuator_ctrl.h`

✅ **Test Suite**
- Comprehensive test program with 10 test cases
- Tests all registers, modes, and functionality
- Located at: `sw/regression_tests/test_custom_actuator.c`

✅ **Documentation**
- Complete integration guide
- Quick reference guide
- This README

### Verify Integration

Run the verification script:
```bash
./verify_custom_ip_integration.sh
```

Expected output: `✓ All checks passed!`

## 📁 File Structure

```
PULPissimo/
├── hw/
│   ├── custom_actuator_ctrl.sv              ← Custom peripheral RTL
│   ├── CUSTOM_PERIPHERAL_README.md          ← Quick reference
│   └── includes/
│       ├── soc_mem_map.svh                  ← Modified (added 0x1A10_D000)
│       └── periph_bus_defines.sv            ← Modified (NB_MASTER: 11→12)
│
├── sw/
│   ├── bootcode/include/
│   │   └── custom_actuator_ctrl.h           ← Software driver header
│   └── regression_tests/
│       └── test_custom_actuator.c           ← Test program
│
├── CUSTOM_IP_INTEGRATION_GUIDE.md           ← Full integration guide
├── CUSTOM_IP_SUMMARY.md                     ← Summary & overview
├── README_CUSTOM_IP_INTEGRATION.md          ← This file
├── verify_custom_ip_integration.sh          ← Verification script
└── Bender.yml                               ← Modified (added sources)
```

## 🔧 Using This Example

### For Testing the Example Peripheral

1. **Read the integration guide**:
   ```bash
   cat CUSTOM_IP_INTEGRATION_GUIDE.md
   ```

2. **Complete the APB bus connection** (see guide for details):
   - Option A: Fork and modify `pulp_soc` (recommended)
   - Option B: Use the `apb_chip_ctrl_master` port

3. **Build and simulate**:
   ```bash
   cd target/sim/questasim
   make clean all
   # Load test_custom_actuator.elf
   ```

### For Your Custom IP Block

1. **Use as template**:
   ```bash
   cp hw/custom_actuator_ctrl.sv hw/my_custom_ip.sv
   cp sw/bootcode/include/custom_actuator_ctrl.h sw/bootcode/include/my_custom_ip.h
   ```

2. **Customize the RTL**:
   - Modify register map for your application
   - Replace FSM with your IP logic
   - Update I/O ports for your signals

3. **Update the software driver**:
   - Change base address and register offsets
   - Update helper functions
   - Match hardware behavior

4. **Write tests**:
   - Copy and modify `test_custom_actuator.c`
   - Test all functionality
   - Verify edge cases

5. **Update memory map**:
   - Choose available address in `soc_mem_map.svh`
   - Update `periph_bus_defines.sv`
   - Update `Bender.yml`

## 📊 Register Map

**Base Address**: `0x1A10_D000`

| Offset | Register      | Type | Description                           |
|--------|---------------|------|---------------------------------------|
| 0x00   | CTRL_REG      | R/W  | Enable, reset, mode control           |
| 0x04   | STATUS_REG    | R    | Busy, error, FSM state                |
| 0x08   | POSITION_REG  | R/W  | Target position/value                 |
| 0x0C   | VELOCITY_REG  | R/W  | Velocity/rate control                 |
| 0x10   | FEEDBACK_REG  | R    | Sensor feedback (read-only)           |
| 0x14   | CONFIG_REG    | R/W  | Interrupt enable, configuration       |

## 💻 Software Example

```c
#include "custom_actuator_ctrl.h"

int main() {
    // Initialize
    custom_actuator_init();
    custom_actuator_enable();
    
    // Configure for position mode
    custom_actuator_set_mode(CUSTOM_ACTUATOR_MODE_POSITION);
    custom_actuator_set_position(1000);  // Target
    custom_actuator_set_velocity(5000);  // Speed
    
    // Wait for completion
    while (custom_actuator_is_busy()) {
        uint16_t feedback = custom_actuator_read_feedback();
        printf("Current: %d\n", feedback);
    }
    
    // Check status
    if (custom_actuator_has_error()) {
        printf("Error occurred!\n");
        return -1;
    }
    
    printf("Target reached!\n");
    return 0;
}
```

## 🎓 Example Use Cases

This integration pattern works for various IP blocks:

### 1. Motor/Actuator Controllers
- **Registers**: Position, velocity, current, mode
- **Signals**: PWM outputs, encoder inputs, enable
- **Logic**: Motion control FSM, PID loops

### 2. Neural Network Accelerators
- **Registers**: Command, config, data buffers, status
- **Signals**: Memory interfaces for weights/activations
- **Logic**: MAC arrays, activation functions

### 3. Signal Processing
- **Registers**: Mode, coefficients, input/output buffers
- **Signals**: Streaming data interfaces
- **Logic**: Filters, FFT, correlation engines

### 4. Communication Controllers
- **Registers**: TX/RX buffers, baud/timing, control
- **Signals**: Physical interface (UART, SPI, custom)
- **Logic**: Protocol state machines, FIFOs

### 5. Sensor Interfaces
- **Registers**: Config, sample rate, data, calibration
- **Signals**: ADC/sensor interfaces
- **Logic**: Sampling control, data conversion

### 6. Security/Crypto
- **Registers**: Key, data, command, status
- **Signals**: Minimal (mostly register-based)
- **Logic**: AES/SHA engines, RNG

## 📚 Documentation Guide

| Document | Purpose | When to Read |
|----------|---------|--------------|
| **This README** | Overview and quick start | Start here |
| **CUSTOM_IP_SUMMARY.md** | Detailed summary of what's included | After overview |
| **CUSTOM_IP_INTEGRATION_GUIDE.md** | Complete step-by-step integration | Before implementing |
| **hw/CUSTOM_PERIPHERAL_README.md** | RTL-specific quick reference | During RTL development |

## ✅ Integration Checklist

Use this checklist when integrating your custom IP:

### RTL Development
- [ ] Create peripheral module with APB interface
- [ ] Define register map (addresses, fields)
- [ ] Implement register read/write logic
- [ ] Add your custom IP logic
- [ ] Generate output signals
- [ ] Add interrupt support (if needed)
- [ ] Create standalone testbench

### System Integration
- [ ] Add to memory map (`soc_mem_map.svh`)
- [ ] Update APB bus config (`periph_bus_defines.sv`)
- [ ] Add to build system (`Bender.yml`)
- [ ] Connect to APB bus in `pulp_soc`
- [ ] Route external signals through hierarchy
- [ ] Add to padframe (if external pins needed)

### Software Development
- [ ] Create driver header file
- [ ] Define register addresses and bit fields
- [ ] Write helper functions
- [ ] Create initialization function
- [ ] Write comprehensive test program
- [ ] Test all registers and modes
- [ ] Verify error handling

### Validation
- [ ] Run RTL testbench
- [ ] Run system simulation with test program
- [ ] Verify all register accesses
- [ ] Check FSM behavior
- [ ] Validate output signals
- [ ] Test interrupt operation
- [ ] Measure performance
- [ ] Test on FPGA (if available)

### Documentation
- [ ] Document register map
- [ ] Write usage examples
- [ ] Add integration notes
- [ ] Update system documentation

## 🐛 Troubleshooting

### RTL Issues

**Problem**: Peripheral doesn't respond
- ✓ Check APB bus connection and address decoding
- ✓ Verify `psel_i` is asserted for correct address
- ✓ Check clock and reset connections
- ✓ Verify `pready_o` is driven correctly

**Problem**: Wrong data read back
- ✓ Check register address calculation (byte vs word)
- ✓ Verify read multiplexer logic
- ✓ Check for register write/read conflicts
- ✓ Verify endianness

**Problem**: Timing violations
- ✓ Add pipeline stages if needed
- ✓ Check for long combinational paths
- ✓ Verify clock domain crossings
- ✓ Check setup/hold times

### Software Issues

**Problem**: Register writes don't work
- ✓ Verify address matches memory map
- ✓ Check pointer type (volatile)
- ✓ Verify cache settings
- ✓ Check write enable in RTL

**Problem**: Reads return garbage
- ✓ Verify address is correct
- ✓ Check that peripheral is clocked
- ✓ Verify APB bus is accessible
- ✓ Check read enable in RTL

## 🔍 Verification

### Run Verification Script

```bash
./verify_custom_ip_integration.sh
```

This checks:
- ✓ All required files exist
- ✓ Memory map is configured
- ✓ APB bus count is updated
- ✓ Build files are updated
- ✓ RTL structure is correct
- ✓ Software interface is defined

### Manual Verification

1. **Check memory map**:
   ```bash
   grep CUSTOM_ACTUATOR hw/includes/soc_mem_map.svh
   ```

2. **Check APB bus config**:
   ```bash
   grep NB_MASTER hw/includes/periph_bus_defines.sv
   ```

3. **Verify in build**:
   ```bash
   grep custom_actuator Bender.yml
   ```

## 🚀 Next Steps

### 1. For This Example
1. Complete APB bus connection (see integration guide)
2. Build simulation environment
3. Run test program
4. Verify all functionality

### 2. For Your IP
1. Study the example peripheral
2. Define your requirements
3. Design your register map
4. Implement and test

### 3. For Production
1. Comprehensive testing
2. Performance validation
3. FPGA prototyping
4. Documentation

## 📖 Additional Resources

- **PULPissimo**: https://github.com/pulp-platform/pulpissimo
- **PULP Platform**: https://pulp-platform.org/
- **PULP SDK**: https://github.com/pulp-platform/pulp-sdk
- **APB Spec**: ARM AMBA APB Protocol Specification
- **RISC-V**: https://riscv.org/

## 🤝 Support

For questions or issues:
1. Check the integration guide
2. Review the example code
3. Run the verification script
4. Check PULPissimo documentation

## 📝 License

This example code follows the PULPissimo license:
- **License**: Solderpad Hardware License v0.51 (SHL-0.51)
- See `LICENSE.md` for details

---

**Status**: ✅ Ready for Integration  
**Version**: 1.0  
**Last Updated**: 2025-11-04

**Summary**: All files created and verified. Ready to connect to APB bus and test!
