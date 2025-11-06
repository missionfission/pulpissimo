# PULP RISC-V IP Features Summary

## Overview

This document summarizes the available features in PULPissimo's RISC-V cores that are relevant for motor control and control-by-wire applications.

## Core Options

PULPissimo supports two RISC-V core options:

### RI5CY (CV32E40P)
- **Pipeline**: 4-stage in-order, single-issue
- **IPC**: Close to 1
- **ISA Extensions**: RV32I, RV32C, RV32M, RV32F (optional)
- **PULP Extensions**: Hardware loops, post-increment loads/stores, MAC operations, SIMD, dot product
- **Best for**: Signal processing, DSP operations, motor control algorithms

### Ibex (Zero-riscy)
- **Pipeline**: 2-stage in-order, single-issue
- **ISA Extensions**: RV32I, RV32C, RV32M (optional), RV32E (optional)
- **Best for**: Ultra-low-power, minimal area applications

## PULP-Specific ISA Extensions

### Hardware Loops
- **CSR Registers**: 
  - `CSR_HWLOOP0_START` (0x800)
  - `CSR_HWLOOP0_END` (0x801)
  - `CSR_HWLOOP0_COUNTER` (0x802)
  - `CSR_HWLOOP1_START` (0x804)
  - `CSR_HWLOOP1_END` (0x805)
  - `CSR_HWLOOP1_COUNTER` (0x806)
- **Usage**: Zero-overhead loops for control algorithms
- **Benefit**: Eliminates loop overhead, critical for high-frequency control loops

### Post-Increment Load/Store Instructions
- **Instructions**: `p.lw`, `p.sw` (and variants)
- **Usage**: Efficient array access in control loops
- **Benefit**: Reduces instruction count, improves loop performance

### MAC Operations
- **Builtins**: `__MAC`, `__MSU`, `__MACS`, `__MACU`, `__MACHHS`, `__MACHHU`
- **Usage**: Multiply-accumulate for PID controllers, filters
- **Benefit**: Single-cycle MAC operations, essential for control algorithms

### SIMD Instructions
- **Types**: v2s, v2u (2x16-bit), v4s, v4u (4x8-bit)
- **Operations**: Add, subtract, multiply, dot product, min/max, clip
- **Usage**: Parallel processing of multiple control channels
- **Benefit**: Process multiple motors/actuators simultaneously

### Dot Product
- **Builtins**: `__DOTP2`, `__DOTPU2`, `__DOTPUS2`
- **Usage**: Vector operations for FOC (Field-Oriented Control)
- **Benefit**: Hardware-accelerated vector math

## Performance Counters

### Cycle Counter
- **CSR**: `CSR_CYCLE` (0xC00), `CSR_CYCLEH` (0xC80)
- **Usage**: Precise timing measurements for control loops
- **Benefit**: Cycle-accurate performance analysis

### Instruction Counter
- **CSR**: `CSR_INSTRET` (0xC02), `CSR_INSTRETH` (0xC82)
- **Usage**: Instruction count profiling
- **Benefit**: Code optimization analysis

## Peripheral Subsystem

### uDMA (micro-DMA)
- **Purpose**: Autonomous I/O transfers
- **Usage**: High-speed data transfer without CPU intervention
- **Benefit**: Offloads CPU, enables high-frequency control loops
- **Interfaces**: SPI, I2C, I2S, UART, Camera Interface (CPI), Hyperbus

### Timers
- **APB Timer**: General-purpose timer
- **Advanced Timer**: More features, multiple channels
- **Usage**: Periodic control loop triggers, PWM generation (software)
- **Benefit**: Deterministic timing for control loops

### GPIO
- **Count**: Configurable (typically 32)
- **Usage**: Digital I/O, simple PWM, encoder inputs
- **Limitation**: Software PWM has jitter, not suitable for high-frequency control

### Interrupt Controller
- **Features**: Multi-level interrupt handling
- **Usage**: Real-time event handling
- **Benefit**: Fast response to control events

## Memory Map

### L2 Memory (TCDM)
- **Address**: 0x1C01_0000 - 0x1C09_0000
- **Usage**: Data storage for control algorithms
- **Access**: Fast access for control loop data

### Boot ROM
- **Address**: 0x1A00_0000 - 0x1A04_0000
- **Usage**: Boot code, can include custom IP drivers

### Peripheral Region
- **Address**: 0x1A10_0000 - 0x1A40_0000
- **Custom Actuator**: 0x1A10_D000 - 0x1A10_E000
- **Usage**: Memory-mapped peripherals

## Hardware Processing Engines (HWPEs)

- **Purpose**: Hardware accelerators for specialized tasks
- **Integration**: Memory-mapped, shared memory with core
- **Example**: MAC engine for vector operations
- **Usage**: Can be used for motor control acceleration

## Compiler Support

### Architecture Flags
- **Base**: `-march=rv32imc -mabi=ilp32`
- **PULP Extensions**: Automatically enabled when using PULP toolchain
- **Optimization**: `-O2`, `-O3`, `-Ofast` supported

### Builtin Functions
- Available via `#include "archi/riscv/builtins_v2.h"`
- Compiler automatically generates optimized code
- Zero overhead when properly used

## Real-Time Capabilities

### Deterministic Execution
- In-order pipeline ensures deterministic timing
- No branch prediction reduces jitter
- Hardware loops eliminate loop overhead

### Interrupt Latency
- Fast interrupt response
- Low-latency interrupt controller
- Suitable for real-time control applications

### Limitations
- Single-core limits parallel processing
- Software PWM has jitter
- No hardware floating-point in Ibex (optional in RI5CY)

## Recommendations for Motor Control

### Use Hardware Loops
- Zero-overhead loops for control algorithms
- Critical for high-frequency control loops (1-20 kHz)

### Leverage MAC Operations
- Hardware MAC for PID controllers
- Single-cycle multiply-accumulate operations

### Use Performance Counters
- Cycle counters for timing measurements
- Essential for benchmarking and optimization

### Consider SIMD
- Parallel processing of multiple control channels
- Useful for multi-motor systems

### Use uDMA
- Offload data transfer from CPU
- Enables higher control loop frequencies

## Gaps Requiring Custom IP

### Hardware PWM Generation
- Current: Software PWM via GPIO (high jitter)
- Need: Hardware PWM with dead-time, synchronized channels

### Encoder Interface
- Current: Software decoding via GPIO
- Need: Hardware quadrature decoder, position counter

### Current Sensing
- Current: No dedicated interface
- Need: ADC interface, overcurrent protection

### Trajectory Planning
- Current: Software-based
- Need: Hardware trajectory generator

### PID Controller
- Current: Software implementation
- Need: Hardware PID for faster control loops

### Safety Features
- Current: Basic error handling
- Need: Watchdog, redundant paths, fault injection

