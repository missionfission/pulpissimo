// Generic FPU register bus stub for Verilator simulation
module pa_fpu_frbus (
  input  ctrl_frbus_ex2_wb_req,
  input  [31:0] dp_frbus_ex2_data,
  input  [4:0] dp_frbus_ex2_fflags,
  input  [31:0] fdsu_frbus_data,
  input  [4:0] fdsu_frbus_fflags,
  input  fdsu_frbus_wb_vld,
  output [31:0] fpu_idu_fwd_data,
  output [4:0] fpu_idu_fwd_fflags,
  output fpu_idu_fwd_vld
);

  // Simple mux - prefer fdsu if valid, else dp
  assign fpu_idu_fwd_vld = fdsu_frbus_wb_vld | ctrl_frbus_ex2_wb_req;
  assign fpu_idu_fwd_data = fdsu_frbus_wb_vld ? fdsu_frbus_data : dp_frbus_ex2_data;
  assign fpu_idu_fwd_fflags = fdsu_frbus_wb_vld ? fdsu_frbus_fflags : dp_frbus_ex2_fflags;

endmodule

