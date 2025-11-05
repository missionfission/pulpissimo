//-----------------------------------------------------------------------------
// Title         : Custom Actuator Controller - Software Interface
//-----------------------------------------------------------------------------
// File          : custom_actuator_ctrl.h
// Author        : Custom IP Integration Example
// Created       : 2025-11-04
//-----------------------------------------------------------------------------
// Description :
// C header file for accessing the custom actuator controller peripheral
// from PULPissimo software. This demonstrates how to create a memory-mapped
// driver for your custom IP block.
//-----------------------------------------------------------------------------
// Copyright (C) 2025 Custom IP Example
// SPDX-License-Identifier: SHL-0.51
//-----------------------------------------------------------------------------

#ifndef __CUSTOM_ACTUATOR_CTRL_H__
#define __CUSTOM_ACTUATOR_CTRL_H__

#include <stdint.h>

//-----------------------------------------------------------------------------
// Base Address
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_BASE_ADDR  0x1A10D000

//-----------------------------------------------------------------------------
// Register Offsets
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_CTRL_REG_OFFSET      0x00
#define CUSTOM_ACTUATOR_STATUS_REG_OFFSET    0x04
#define CUSTOM_ACTUATOR_POSITION_REG_OFFSET  0x08
#define CUSTOM_ACTUATOR_VELOCITY_REG_OFFSET  0x0C
#define CUSTOM_ACTUATOR_FEEDBACK_REG_OFFSET  0x10
#define CUSTOM_ACTUATOR_CONFIG_REG_OFFSET    0x14

//-----------------------------------------------------------------------------
// Register Addresses
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_CTRL_REG     (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_CTRL_REG_OFFSET))
#define CUSTOM_ACTUATOR_STATUS_REG   (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_STATUS_REG_OFFSET))
#define CUSTOM_ACTUATOR_POSITION_REG (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_POSITION_REG_OFFSET))
#define CUSTOM_ACTUATOR_VELOCITY_REG (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_VELOCITY_REG_OFFSET))
#define CUSTOM_ACTUATOR_FEEDBACK_REG (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_FEEDBACK_REG_OFFSET))
#define CUSTOM_ACTUATOR_CONFIG_REG   (*(volatile uint32_t*)(CUSTOM_ACTUATOR_BASE_ADDR + CUSTOM_ACTUATOR_CONFIG_REG_OFFSET))

//-----------------------------------------------------------------------------
// Control Register Bit Definitions
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_CTRL_ENABLE_BIT     (1 << 0)  // Enable actuator
#define CUSTOM_ACTUATOR_CTRL_RESET_BIT      (1 << 1)  // Software reset
#define CUSTOM_ACTUATOR_CTRL_MODE_SHIFT     4         // Operating mode field
#define CUSTOM_ACTUATOR_CTRL_MODE_MASK      (0xF << CUSTOM_ACTUATOR_CTRL_MODE_SHIFT)

// Operating Modes
#define CUSTOM_ACTUATOR_MODE_IDLE           0x0
#define CUSTOM_ACTUATOR_MODE_POSITION       0x1
#define CUSTOM_ACTUATOR_MODE_VELOCITY       0x2
#define CUSTOM_ACTUATOR_MODE_TORQUE         0x3

//-----------------------------------------------------------------------------
// Status Register Bit Definitions
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_STATUS_BUSY_BIT     (1 << 0)  // Operation in progress
#define CUSTOM_ACTUATOR_STATUS_ERROR_BIT    (1 << 1)  // Error flag
#define CUSTOM_ACTUATOR_STATUS_STATE_SHIFT  8         // FSM state field
#define CUSTOM_ACTUATOR_STATUS_STATE_MASK   (0xFF << CUSTOM_ACTUATOR_STATUS_STATE_SHIFT)

//-----------------------------------------------------------------------------
// Config Register Bit Definitions
//-----------------------------------------------------------------------------
#define CUSTOM_ACTUATOR_CONFIG_IRQ_EN_BIT   (1 << 0)  // Interrupt enable

//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------

/**
 * @brief Initialize the custom actuator controller
 */
static inline void custom_actuator_init(void) {
    CUSTOM_ACTUATOR_CTRL_REG = 0;  // Reset control register
    CUSTOM_ACTUATOR_CONFIG_REG = 0; // Disable interrupts
}

/**
 * @brief Enable the actuator
 */
static inline void custom_actuator_enable(void) {
    CUSTOM_ACTUATOR_CTRL_REG |= CUSTOM_ACTUATOR_CTRL_ENABLE_BIT;
}

/**
 * @brief Disable the actuator
 */
static inline void custom_actuator_disable(void) {
    CUSTOM_ACTUATOR_CTRL_REG &= ~CUSTOM_ACTUATOR_CTRL_ENABLE_BIT;
}

/**
 * @brief Software reset of the actuator controller
 */
static inline void custom_actuator_reset(void) {
    CUSTOM_ACTUATOR_CTRL_REG |= CUSTOM_ACTUATOR_CTRL_RESET_BIT;
}

/**
 * @brief Set operating mode
 * @param mode Operating mode (0-15)
 */
static inline void custom_actuator_set_mode(uint8_t mode) {
    uint32_t ctrl = CUSTOM_ACTUATOR_CTRL_REG;
    ctrl &= ~CUSTOM_ACTUATOR_CTRL_MODE_MASK;
    ctrl |= ((mode & 0xF) << CUSTOM_ACTUATOR_CTRL_MODE_SHIFT);
    CUSTOM_ACTUATOR_CTRL_REG = ctrl;
}

/**
 * @brief Set target position
 * @param position Target position value
 */
static inline void custom_actuator_set_position(uint32_t position) {
    CUSTOM_ACTUATOR_POSITION_REG = position;
}

/**
 * @brief Set velocity/rate
 * @param velocity Velocity control value
 */
static inline void custom_actuator_set_velocity(uint32_t velocity) {
    CUSTOM_ACTUATOR_VELOCITY_REG = velocity;
}

/**
 * @brief Read current feedback/sensor value
 * @return Sensor feedback value
 */
static inline uint16_t custom_actuator_read_feedback(void) {
    return (uint16_t)(CUSTOM_ACTUATOR_FEEDBACK_REG & 0xFFFF);
}

/**
 * @brief Check if actuator is busy
 * @return 1 if busy, 0 if idle
 */
static inline int custom_actuator_is_busy(void) {
    return (CUSTOM_ACTUATOR_STATUS_REG & CUSTOM_ACTUATOR_STATUS_BUSY_BIT) ? 1 : 0;
}

/**
 * @brief Check if error occurred
 * @return 1 if error, 0 if no error
 */
static inline int custom_actuator_has_error(void) {
    return (CUSTOM_ACTUATOR_STATUS_REG & CUSTOM_ACTUATOR_STATUS_ERROR_BIT) ? 1 : 0;
}

/**
 * @brief Get current FSM state
 * @return Current state value
 */
static inline uint8_t custom_actuator_get_state(void) {
    return (uint8_t)((CUSTOM_ACTUATOR_STATUS_REG & CUSTOM_ACTUATOR_STATUS_STATE_MASK) 
                     >> CUSTOM_ACTUATOR_STATUS_STATE_SHIFT);
}

/**
 * @brief Enable interrupts
 */
static inline void custom_actuator_irq_enable(void) {
    CUSTOM_ACTUATOR_CONFIG_REG |= CUSTOM_ACTUATOR_CONFIG_IRQ_EN_BIT;
}

/**
 * @brief Disable interrupts
 */
static inline void custom_actuator_irq_disable(void) {
    CUSTOM_ACTUATOR_CONFIG_REG &= ~CUSTOM_ACTUATOR_CONFIG_IRQ_EN_BIT;
}

/**
 * @brief Read status register
 * @return Full status register value
 */
static inline uint32_t custom_actuator_read_status(void) {
    return CUSTOM_ACTUATOR_STATUS_REG;
}

#endif // __CUSTOM_ACTUATOR_CTRL_H__
