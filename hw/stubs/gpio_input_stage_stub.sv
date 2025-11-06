// Generic GPIO input stage stub for Verilator simulation
module gpio_input_stage #(
  parameter int unsigned NrSyncStages = 2
) (
  input  logic clk_i,
  input  logic rst_ni,
  input  logic en_i,
  input  logic serial_i,
  output logic r_edge_o,
  output logic f_edge_o,
  output logic serial_o
);

  logic [NrSyncStages-1:0] sync_ff;

  // Simple synchronizer
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      sync_ff <= '0;
    end else begin
      sync_ff <= {sync_ff[NrSyncStages-2:0], serial_i};
    end
  end

  assign serial_o = sync_ff[NrSyncStages-1];
  assign r_edge_o = en_i && serial_o && !sync_ff[NrSyncStages-2];
  assign f_edge_o = en_i && !serial_o && sync_ff[NrSyncStages-2];

endmodule


