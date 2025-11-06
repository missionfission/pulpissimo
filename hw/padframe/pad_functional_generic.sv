// Generic pad cells for Verilator simulation
// Simplified versions for RTL simulation

module pad_functional_pd
(
   input  logic             OEN,
   input  logic             I,
   output logic             O,
   input  logic             PEN,
   inout  logic             PAD
);

  assign O = PAD;
  assign PAD = OEN ? 1'bz : I;

endmodule

module pad_functional_pu
(
   input  logic             OEN,
   input  logic             I,
   output logic             O,
   input  logic             PEN,
   inout  logic             PAD
);

  assign O = PAD;
  assign PAD = OEN ? 1'bz : I;

endmodule


