// Copyright 2024 Custom IP Integration
// DPI-C interface for debug bus access without force statements
// This allows Verilator-compatible memory loading

package debug_bus_dpi_pkg;

  // DPI-C import for C++ side (Verilator)
  import "DPI-C" context function void debug_bus_write_word(
    input int unsigned addr,
    input int unsigned data
  );

  import "DPI-C" context function int unsigned debug_bus_read_word(
    input int unsigned addr
  );

  import "DPI-C" context function void debug_bus_init();

  import "DPI-C" context function void debug_bus_finish();

  // Task wrapper for SystemVerilog side
  task automatic debug_bus_write(
    input logic [31:0] addr,
    input logic [31:0] data,
    ref logic clk
  );
    debug_bus_write_word(addr, data);
    @(posedge clk);
  endtask

  task automatic debug_bus_read(
    input logic [31:0] addr,
    output logic [31:0] data,
    ref logic clk
  );
    data = debug_bus_read_word(addr);
    @(posedge clk);
  endtask

endpackage

// Module wrapper for debug bus access
// This module can be instantiated in the testbench to drive the debug bus
module debug_bus_driver (
  input  logic        clk_i,
  input  logic        rst_ni,
  
  // Debug bus master interface (drives the bus)
  output logic        debug_req_o,
  output logic [31:0] debug_add_o,
  output logic        debug_wen_o,
  output logic [31:0] debug_wdata_o,
  output logic [3:0]  debug_be_o,
  input  logic        debug_gnt_i,
  input  logic [31:0] debug_rdata_i,
  input  logic        debug_r_valid_i
);

  import debug_bus_dpi_pkg::*;

  typedef enum logic [1:0] {
    IDLE,
    WAIT_GRANT,
    WAIT_VALID
  } state_e;

  state_e state;
  logic [31:0] pending_addr;
  logic [31:0] pending_data;
  logic pending_write;
  logic [31:0] read_data;

  initial begin
    debug_bus_init();
  end

  final begin
    debug_bus_finish();
  end

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      state <= IDLE;
      debug_req_o <= 1'b0;
      debug_add_o <= 32'h0;
      debug_wen_o <= 1'b1; // Default to read
      debug_wdata_o <= 32'h0;
      debug_be_o <= 4'hF;
      pending_addr <= 32'h0;
      pending_data <= 32'h0;
      pending_write <= 1'b0;
      read_data <= 32'h0;
    end else begin
      case (state)
        IDLE: begin
          // Check if DPI function wants to write
          // This is a placeholder - actual implementation would use a queue
          // or shared memory between DPI and SystemVerilog
          debug_req_o <= 1'b0;
        end
        
        WAIT_GRANT: begin
          if (debug_gnt_i) begin
            state <= WAIT_VALID;
          end
        end
        
        WAIT_VALID: begin
          if (debug_r_valid_i) begin
            if (!pending_write) begin
              read_data <= debug_rdata_i;
            end
            state <= IDLE;
            debug_req_o <= 1'b0;
          end
        end
        
        default: state <= IDLE;
      endcase
    end
  end

  // Task to write a word
  task write_word(input logic [31:0] addr, input logic [31:0] data);
    pending_addr <= addr;
    pending_data <= data;
    pending_write <= 1'b1;
    debug_req_o <= 1'b1;
    debug_add_o <= addr;
    debug_wdata_o <= data;
    debug_wen_o <= 1'b0; // Write
    debug_be_o <= 4'hF;
    state <= WAIT_GRANT;
    @(posedge clk_i);
    while (state != IDLE) @(posedge clk_i);
  endtask

  // Task to read a word
  task read_word(input logic [31:0] addr, output logic [31:0] data);
    pending_addr <= addr;
    pending_write <= 1'b0;
    debug_req_o <= 1'b1;
    debug_add_o <= addr;
    debug_wen_o <= 1'b1; // Read
    debug_be_o <= 4'hF;
    state <= WAIT_GRANT;
    @(posedge clk_i);
    while (state != IDLE) @(posedge clk_i);
    data = read_data;
  endtask

endmodule

