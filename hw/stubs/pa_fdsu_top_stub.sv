// Generic FPU divide/sqrt unit stub for Verilator simulation
module pa_fdsu_top (
  input  cp0_fpu_icg_en,
  input  cp0_fpu_xx_dqnan,
  input  cp0_yy_clk_en,
  input  cpurst_b,
  input  ctrl_fdsu_ex1_sel,
  input  ctrl_xx_ex1_cmplt_dp,
  input  ctrl_xx_ex1_inst_vld,
  input  ctrl_xx_ex1_stall,
  input  ctrl_xx_ex1_warm_up,
  input  ctrl_xx_ex2_warm_up,
  input  ctrl_xx_ex3_warm_up,
  input  dp_xx_ex1_cnan,
  input  [4:0] dp_xx_ex1_id,
  input  dp_xx_ex1_inf,
  input  dp_xx_ex1_qnan,
  input  [2:0] dp_xx_ex1_rm,
  input  dp_xx_ex1_snan,
  input  dp_xx_ex1_zero,
  output [63:0] fdsu_fpu_debug_info,
  output fdsu_fpu_ex1_cmplt,
  output fdsu_fpu_ex1_cmplt_dp,
  output [4:0] fdsu_fpu_ex1_fflags,
  output [2:0] fdsu_fpu_ex1_special_sel,
  output fdsu_fpu_ex1_special_sign,
  output fdsu_fpu_ex1_stall,
  output fdsu_fpu_no_op,
  output [31:0] fdsu_frbus_data,
  output [4:0] fdsu_frbus_fflags,
  output [4:0] fdsu_frbus_freg,
  output fdsu_frbus_wb_vld,
  input  forever_cpuclk,
  input  frbus_fdsu_wb_grant,
  input  [4:0] idu_fpu_ex1_dst_freg,
  input  [2:0] idu_fpu_ex1_eu_sel,
  input  [9:0] idu_fpu_ex1_func,
  input  [31:0] idu_fpu_ex1_srcf0,
  input  [31:0] idu_fpu_ex1_srcf1,
  input  pad_yy_icg_scan_en,
  input  rtu_xx_ex1_cancel,
  input  rtu_xx_ex2_cancel,
  input  rtu_yy_xx_async_flush,
  input  rtu_yy_xx_flush
);

  // Simple stub - always ready, no operation
  assign fdsu_fpu_ex1_cmplt = 1'b0;
  assign fdsu_fpu_ex1_cmplt_dp = 1'b0;
  assign fdsu_fpu_ex1_fflags = 5'b0;
  assign fdsu_fpu_ex1_special_sel = 3'b0;
  assign fdsu_fpu_ex1_special_sign = 1'b0;
  assign fdsu_fpu_ex1_stall = 1'b0;
  assign fdsu_fpu_no_op = 1'b1;
  assign fdsu_frbus_data = 32'h0;
  assign fdsu_frbus_fflags = 5'b0;
  assign fdsu_frbus_freg = 5'b0;
  assign fdsu_frbus_wb_vld = 1'b0;
  assign fdsu_fpu_debug_info = 64'h0;

endmodule

