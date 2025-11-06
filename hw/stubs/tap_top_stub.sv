// Generic JTAG TAP stub for Verilator simulation
module tap_top #(
    parameter IDCODE_VALUE = 32'h10000db3
)(
  input  tms_i,
  input  tck_i,
  input  rst_ni,
  input  td_i,
  output td_o,
  output shift_dr_o,
  output update_dr_o,
  output capture_dr_o,
  output memory_sel_o,
  output fifo_sel_o,
  output confreg_sel_o,
  output clk_byp_sel_o,
  output observ_sel_o,
  output scan_in_o,
  input  memory_out_i,
  input  fifo_out_i,
  input  confreg_out_i,
  input  clk_byp_out_i,
  input  observ_out_i
);

  // Simple stub - minimal JTAG functionality
  assign shift_dr_o = 1'b0;
  assign update_dr_o = 1'b0;
  assign capture_dr_o = 1'b0;
  assign memory_sel_o = 1'b0;
  assign fifo_sel_o = 1'b0;
  assign confreg_sel_o = 1'b0;
  assign clk_byp_sel_o = 1'b0;
  assign observ_sel_o = 1'b0;
  assign scan_in_o = td_i;
  assign td_o = memory_out_i; // Simple pass-through

endmodule


