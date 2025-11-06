// Generic clock gate stub for Verilator simulation
module cv32e40p_clock_gate (
  input  logic clk_i,
  input  logic en_i,
  input  logic scan_cg_en_i,
  output logic clk_o
);

  // Simple clock gate - pass through when enabled
  assign clk_o = scan_cg_en_i ? clk_i : (en_i ? clk_i : 1'b0);

endmodule

