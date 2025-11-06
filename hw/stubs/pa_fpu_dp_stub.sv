// Generic FPU datapath stub for Verilator simulation
module pa_fpu_dp (
  input  cp0_fpu_icg_en,
  input  [2:0] cp0_fpu_xx_rm,
  input  cp0_yy_clk_en,
  input  ctrl_xx_ex1_inst_vld,
  input  ctrl_xx_ex1_stall,
  input  ctrl_xx_ex1_warm_up,
  output [31:0] dp_frbus_ex2_data,
  output [4:0] dp_frbus_ex2_fflags,
  output dp_xx_ex1_cnan,
  output [4:0] dp_xx_ex1_id,
  output dp_xx_ex1_inf,
  output dp_xx_ex1_norm,
  output dp_xx_ex1_qnan,
  output dp_xx_ex1_snan,
  output dp_xx_ex1_zero,
  output ex2_inst_wb,
  input  [4:0] fdsu_fpu_ex1_fflags,
  input  [2:0] fdsu_fpu_ex1_special_sel,
  input  fdsu_fpu_ex1_special_sign,
  input  forever_cpuclk,
  input  [2:0] idu_fpu_ex1_eu_sel,
  input  [9:0] idu_fpu_ex1_func,
  input  [2:0] idu_fpu_ex1_rm,
  input  [31:0] idu_fpu_ex1_srcf0,
  input  [31:0] idu_fpu_ex1_srcf1,
  input  [31:0] idu_fpu_ex1_srcf2,
  input  idu_fpu_ex1_gateclk_vld,
  input  pad_yy_icg_scan_en
);

  // Simple stub - pass through zeros
  assign dp_frbus_ex2_data = 32'h0;
  assign dp_frbus_ex2_fflags = 5'b0;
  assign dp_xx_ex1_cnan = 1'b0;
  assign dp_xx_ex1_id = 5'b0;
  assign dp_xx_ex1_inf = 1'b0;
  assign dp_xx_ex1_norm = 1'b0;
  assign dp_xx_ex1_qnan = 1'b0;
  assign dp_xx_ex1_snan = 1'b0;
  assign dp_xx_ex1_zero = 1'b0;
  assign ex2_inst_wb = 1'b0;

endmodule

