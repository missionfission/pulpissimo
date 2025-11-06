# Design Decisions for Motor Control Custom IP

## Overview

This document justifies the RTL modifications and design decisions for the enhanced custom actuator controller IP targeting motor control and control-by-wire applications.

## Why Hardware Acceleration?

### Software Limitations

#### Control Loop Frequency
- **Software PWM**: Limited by CPU cycles, typically <5 kHz
- **Hardware PWM**: Can achieve 20-100 kHz with deterministic timing
- **Requirement**: 10-50 kHz for current control loops
- **Conclusion**: Hardware PWM essential for high-performance control

#### Latency
- **Software PID**: 10-50 microseconds typical
- **Hardware PID**: <1 microsecond
- **Requirement**: <10 microseconds for current loops
- **Conclusion**: Hardware PID required for low-latency control

#### Jitter
- **Software Timing**: Variable due to interrupt latency, cache misses
- **Hardware Timing**: Deterministic, cycle-accurate
- **Requirement**: <1% jitter for stable control
- **Conclusion**: Hardware timing essential for deterministic control

### CPU Offloading

#### Computation Offload
- **PID Control**: Offloads CPU-intensive calculations
- **Trajectory Planning**: Reduces CPU load for complex profiles
- **Encoder Decoding**: Eliminates CPU overhead for position tracking
- **Benefit**: CPU available for higher-level control, communication

#### Data Transfer Offload
- **DMA Support**: Streams trajectory data without CPU
- **ADC Interface**: Direct connection reduces CPU load
- **Benefit**: Enables higher control loop frequencies

## RTL Enhancement Justification

### High-Precision PWM Generation

#### Multi-Phase PWM
- **Justification**: 3-phase motors require 6 PWM signals (3 phases × 2 switches)
- **Current State**: Single PWM output in basic IP
- **Enhancement**: 6-channel PWM with synchronization
- **Benefit**: Direct support for 3-phase motor control

#### Dead-Time Insertion
- **Justification**: Prevents shoot-through in bridge circuits (safety critical)
- **Current State**: No dead-time support
- **Enhancement**: Hardware dead-time generator
- **Benefit**: Safe operation, prevents hardware damage

#### PWM Resolution
- **Justification**: Higher resolution enables smoother control
- **Current State**: 16-bit resolution
- **Enhancement**: 32-bit resolution
- **Benefit**: Finer control, reduced quantization error

### Advanced Encoder Processing

#### Quadrature Decoder
- **Justification**: Software decoding limits control loop frequency
- **Current State**: Software decoding via GPIO
- **Enhancement**: Hardware quadrature decoder
- **Benefit**: Higher frequency position updates, reduced CPU load

#### Velocity Estimation
- **Justification**: Velocity feedback essential for control
- **Current State**: Software calculation
- **Enhancement**: Hardware velocity calculation
- **Benefit**: Low-latency velocity feedback

### Current Sensing & Protection

#### ADC Interface
- **Justification**: Current sensing required for current control
- **Current State**: No current sensing support
- **Enhancement**: Direct ADC interface
- **Benefit**: Fast current measurement for control loops

#### Overcurrent Protection
- **Justification**: Safety critical, must be fast
- **Current State**: No protection
- **Enhancement**: Hardware comparator with automatic shutdown
- **Benefit**: Fast fault response, hardware safety

### Trajectory Planning Hardware

#### Profile Generator
- **Justification**: Complex profiles require significant CPU time
- **Current State**: Software-based trajectory planning
- **Enhancement**: Hardware profile generator
- **Benefit**: Offloads CPU, enables complex trajectories

#### DMA Support
- **Justification**: Large trajectories require high data transfer rates
- **Current State**: CPU-managed data transfer
- **Enhancement**: DMA interface for trajectory streaming
- **Benefit**: Enables longer trajectories, reduces CPU load

### Control Loop Acceleration

#### Hardware PID
- **Justification**: Software PID too slow for high-frequency loops
- **Current State**: Software PID implementation
- **Enhancement**: Hardware PID controller
- **Benefit**: Low-latency control, enables higher loop frequencies

#### Cascade Control
- **Justification**: Common control architecture (position-velocity-current)
- **Current State**: Single-loop control
- **Enhancement**: Multi-loop cascade support
- **Benefit**: Better control performance, industry-standard approach

## Control-by-Wire Enhancements

### Safety Features

#### Watchdog Timer
- **Justification**: ISO 26262 / IEC 61508 requirement
- **Current State**: No watchdog support
- **Enhancement**: Independent watchdog timer
- **Benefit**: Detects software lockups, safety compliance

#### Redundant Control Paths
- **Justification**: Safety-critical applications require redundancy
- **Current State**: Single control path
- **Enhancement**: Dual-channel with comparison
- **Benefit**: Fault tolerance, safety compliance

### Real-Time Guarantees

#### Cycle-Accurate Timing
- **Justification**: Deterministic timing required for control-by-wire
- **Current State**: Software timing (variable)
- **Enhancement**: Hardware cycle counters
- **Benefit**: Deterministic execution, timing guarantees

#### Time-Triggered Execution
- **Justification**: Periodic control loops require precise timing
- **Current State**: Interrupt-driven (variable latency)
- **Enhancement**: Hardware timer triggers
- **Benefit**: Deterministic control loop execution

## Integration with PULP Features

### Leveraging Hardware Loops
- **Usage**: Control algorithm loops
- **Benefit**: Zero-overhead loops for control code
- **Integration**: Software uses hardware loops, IP handles hardware acceleration

### Leveraging MAC Operations
- **Usage**: Transform calculations (Clarke/Park)
- **Benefit**: Fast matrix operations
- **Integration**: Software for transforms, IP for control loops

### Leveraging uDMA
- **Usage**: Trajectory data streaming
- **Benefit**: Offloads CPU for data transfer
- **Integration**: IP requests DMA, uDMA handles transfer

### Leveraging Performance Counters
- **Usage**: Timing measurements for benchmarking
- **Benefit**: Cycle-accurate performance analysis
- **Integration**: Software reads counters, IP provides cycle-accurate timing

## Trade-offs

### Area vs Performance
- **Decision**: Prioritize performance for motor control
- **Rationale**: Control performance critical, area acceptable for functionality
- **Impact**: Larger IP, but enables high-performance control

### Flexibility vs Performance
- **Decision**: Configurable IP with hardware acceleration
- **Rationale**: Balance between flexibility and performance
- **Impact**: More registers, but supports various applications

### Software vs Hardware
- **Decision**: Hardware acceleration for critical paths
- **Rationale**: Software too slow for high-frequency control
- **Impact**: More hardware, but enables real-time control

## Register Map Design

### Register Organization
- **Grouping**: Related registers grouped together
- **Alignment**: 32-bit aligned for efficient access
- **Access**: Mix of read-only, write-only, read-write
- **Rationale**: Standard APB peripheral design

### Bit Field Design
- **Encoding**: Efficient bit field encoding
- **Reserved**: Reserved bits for future expansion
- **Rationale**: Allows future enhancements without breaking compatibility

## Interface Design

### Signal Organization
- **Grouping**: Related signals grouped (PWM, encoder, current, safety)
- **Width**: Appropriate widths for signals (e.g., 32-bit for data, 1-bit for control)
- **Rationale**: Clear interface, easy to understand and use

### Clock Domain
- **Decision**: Single clock domain (peripheral clock)
- **Rationale**: Simplifies design, adequate for control frequencies
- **Alternative Considered**: Separate high-speed clock for PWM (rejected for simplicity)

## Testing Strategy

### Unit Testing
- **RTL Simulation**: Test individual blocks
- **Coverage**: High coverage of control paths
- **Rationale**: Ensure correctness before integration

### Integration Testing
- **System Simulation**: Test with PULPissimo
- **Benchmarks**: Compare with software implementation
- **Rationale**: Validate performance improvements

### Safety Testing
- **Fault Injection**: Test fault handling
- **Watchdog Testing**: Validate watchdog operation
- **Rationale**: Ensure safety features work correctly

## Future Enhancements

### Potential Additions
- **FOC Support**: Hardware FOC transforms
- **CAN Interface**: Direct CAN bus support
- **Multi-Motor**: Support for multiple motors
- **Advanced Profiles**: More trajectory profile types

### Scalability
- **Parameterization**: IP parameterized for different configurations
- **Modularity**: Blocks can be enabled/disabled
- **Rationale**: Support various applications without redesign

## Conclusion

The enhanced custom IP design balances performance, safety, and flexibility to enable high-performance motor control and control-by-wire applications on PULPissimo. Hardware acceleration is essential for meeting real-time requirements, while integration with existing PULP features maximizes overall system performance.


