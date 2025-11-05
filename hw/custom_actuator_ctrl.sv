//-----------------------------------------------------------------------------
// Title         : Custom Actuator Controller Peripheral
//-----------------------------------------------------------------------------
// File          : custom_actuator_ctrl.sv
// Author        : Custom IP Integration Example
// Created       : 2025-11-04
//-----------------------------------------------------------------------------
// Description :
// Example custom IP block for PULPissimo integration. This demonstrates how
// to create a memory-mapped peripheral for testing custom actuator control
// logic, signal processing, or other specialized IP.
//
// Register Map (APB):
//   0x00: CTRL_REG      - Control register (R/W)
//         [0]     ENABLE      - Enable actuator
//         [1]     RESET       - Software reset
//         [7:4]   MODE        - Operating mode
//   0x04: STATUS_REG    - Status register (R)
//         [0]     BUSY        - Operation in progress
//         [1]     ERROR       - Error flag
//         [15:8]  STATE       - Current FSM state
//   0x08: POSITION_REG  - Target position/value (R/W)
//   0x0C: VELOCITY_REG  - Velocity/rate control (R/W)
//   0x10: FEEDBACK_REG  - Sensor feedback (R)
//   0x14: CONFIG_REG    - Configuration register (R/W)
//-----------------------------------------------------------------------------
// Copyright (C) 2025 Custom IP Example
// SPDX-License-Identifier: SHL-0.51
//-----------------------------------------------------------------------------

module custom_actuator_ctrl #(
    parameter int unsigned ADDR_WIDTH = 32,
    parameter int unsigned DATA_WIDTH = 32
)(
    // Clock and Reset
    input  logic                  clk_i,
    input  logic                  rst_ni,
    
    // APB Interface
    input  logic                  psel_i,
    input  logic                  penable_i,
    input  logic                  pwrite_i,
    input  logic [ADDR_WIDTH-1:0] paddr_i,
    input  logic [DATA_WIDTH-1:0] pwdata_i,
    output logic [DATA_WIDTH-1:0] prdata_o,
    output logic                  pready_o,
    output logic                  pslverr_o,
    
    // Custom IP Interface - Example actuator control signals
    output logic [15:0]           actuator_pwm_o,     // PWM output
    output logic                  actuator_dir_o,     // Direction
    output logic                  actuator_enable_o,  // Enable signal
    input  logic [15:0]           sensor_feedback_i,  // Position/feedback sensor
    
    // Interrupt output
    output logic                  interrupt_o
);

    // Register addresses (byte-addressed)
    localparam logic [7:0] CTRL_REG_ADDR     = 8'h00;
    localparam logic [7:0] STATUS_REG_ADDR   = 8'h04;
    localparam logic [7:0] POSITION_REG_ADDR = 8'h08;
    localparam logic [7:0] VELOCITY_REG_ADDR = 8'h0C;
    localparam logic [7:0] FEEDBACK_REG_ADDR = 8'h10;
    localparam logic [7:0] CONFIG_REG_ADDR   = 8'h14;

    // Internal registers
    logic [DATA_WIDTH-1:0] ctrl_reg_q,     ctrl_reg_d;
    logic [DATA_WIDTH-1:0] position_reg_q, position_reg_d;
    logic [DATA_WIDTH-1:0] velocity_reg_q, velocity_reg_d;
    logic [DATA_WIDTH-1:0] config_reg_q,   config_reg_d;
    
    // Status register (mostly read-only, updated by hardware)
    logic [DATA_WIDTH-1:0] status_reg;
    logic                  busy_flag;
    logic                  error_flag;
    logic [7:0]            fsm_state;
    
    // APB transaction detection
    logic                  apb_write;
    logic                  apb_read;
    logic [7:0]            reg_addr;
    
    assign apb_write = psel_i & penable_i & pwrite_i;
    assign apb_read  = psel_i & penable_i & ~pwrite_i;
    assign reg_addr  = paddr_i[7:0];
    
    // APB is always ready (no wait states for this simple peripheral)
    assign pready_o  = 1'b1;
    assign pslverr_o = 1'b0;

    //--------------------------------------------------------------------------
    // Register Write Logic
    //--------------------------------------------------------------------------
    always_comb begin
        // Default: hold current values
        ctrl_reg_d     = ctrl_reg_q;
        position_reg_d = position_reg_q;
        velocity_reg_d = velocity_reg_q;
        config_reg_d   = config_reg_q;
        
        if (apb_write) begin
            case (reg_addr)
                CTRL_REG_ADDR:     ctrl_reg_d     = pwdata_i;
                POSITION_REG_ADDR: position_reg_d = pwdata_i;
                VELOCITY_REG_ADDR: velocity_reg_d = pwdata_i;
                CONFIG_REG_ADDR:   config_reg_d   = pwdata_i;
                default: ;
            endcase
        end
        
        // Software reset handling
        if (ctrl_reg_q[1]) begin // RESET bit
            ctrl_reg_d     = '0;
            position_reg_d = '0;
            velocity_reg_d = '0;
        end
    end

    //--------------------------------------------------------------------------
    // Register Read Logic
    //--------------------------------------------------------------------------
    always_comb begin
        prdata_o = '0;
        if (apb_read) begin
            case (reg_addr)
                CTRL_REG_ADDR:     prdata_o = ctrl_reg_q;
                STATUS_REG_ADDR:   prdata_o = status_reg;
                POSITION_REG_ADDR: prdata_o = position_reg_q;
                VELOCITY_REG_ADDR: prdata_o = velocity_reg_q;
                FEEDBACK_REG_ADDR: prdata_o = {16'h0000, sensor_feedback_i};
                CONFIG_REG_ADDR:   prdata_o = config_reg_q;
                default:           prdata_o = 32'hDEADBEEF; // Debug pattern
            endcase
        end
    end

    //--------------------------------------------------------------------------
    // Sequential Logic - Register Updates
    //--------------------------------------------------------------------------
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (!rst_ni) begin
            ctrl_reg_q     <= '0;
            position_reg_q <= '0;
            velocity_reg_q <= '0;
            config_reg_q   <= '0;
        end else begin
            ctrl_reg_q     <= ctrl_reg_d;
            position_reg_q <= position_reg_d;
            velocity_reg_q <= velocity_reg_d;
            config_reg_q   <= config_reg_d;
        end
    end

    //--------------------------------------------------------------------------
    // Custom IP Logic - Example Actuator Control
    //--------------------------------------------------------------------------
    // This is where your custom IP logic would go. This example shows a
    // simple PWM-based actuator controller.
    
    logic [15:0] pwm_counter_q;
    logic [15:0] current_position_q;
    logic [3:0]  state_q, state_d;
    
    // Extract control signals
    logic enable;
    logic [3:0] mode;
    
    assign enable = ctrl_reg_q[0];
    assign mode   = ctrl_reg_q[7:4];
    
    // Simple state machine for demonstration
    localparam IDLE       = 4'h0;
    localparam RUNNING    = 4'h1;
    localparam POSITIONING = 4'h2;
    localparam ERROR      = 4'hE;
    
    always_comb begin
        state_d = state_q;
        busy_flag = 1'b0;
        error_flag = 1'b0;
        
        case (state_q)
            IDLE: begin
                if (enable) state_d = RUNNING;
            end
            
            RUNNING: begin
                busy_flag = 1'b1;
                if (!enable) state_d = IDLE;
                else if (position_reg_q != current_position_q) 
                    state_d = POSITIONING;
            end
            
            POSITIONING: begin
                busy_flag = 1'b1;
                if (current_position_q == position_reg_q)
                    state_d = RUNNING;
            end
            
            ERROR: begin
                error_flag = 1'b1;
                if (!enable) state_d = IDLE;
            end
            
            default: state_d = ERROR;
        endcase
    end
    
    // PWM generation example
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (!rst_ni) begin
            pwm_counter_q <= '0;
            current_position_q <= '0;
            state_q <= IDLE;
        end else begin
            state_q <= state_d;
            pwm_counter_q <= pwm_counter_q + 1'b1;
            
            // Simple position tracking (move toward target)
            if (state_q == POSITIONING) begin
                if (current_position_q < position_reg_q[15:0])
                    current_position_q <= current_position_q + 1'b1;
                else if (current_position_q > position_reg_q[15:0])
                    current_position_q <= current_position_q - 1'b1;
            end
        end
    end
    
    // Output signals
    assign actuator_pwm_o    = (pwm_counter_q < velocity_reg_q[15:0]) ? current_position_q : 16'h0;
    assign actuator_dir_o    = position_reg_q[16]; // Use bit 16 for direction
    assign actuator_enable_o = enable & (state_q == RUNNING || state_q == POSITIONING);
    
    // Status register construction
    assign fsm_state  = {4'h0, state_q};
    assign status_reg = {16'h0, fsm_state, 6'h0, error_flag, busy_flag};
    
    // Interrupt generation (example: trigger on reaching target position)
    logic interrupt_enable;
    assign interrupt_enable = config_reg_q[0];
    assign interrupt_o = interrupt_enable & (state_q == POSITIONING) & 
                         (current_position_q == position_reg_q[15:0]);

endmodule
