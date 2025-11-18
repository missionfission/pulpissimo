git clone https://github.com/pulp-platform/pulpissimo.git

Add version of bender 0.28.0 

/sw/freetools/bender/0.28.0/rh80_64/bender checkout

cp /prj/adgsys/users/prateek/pulp_10nov_trial/pulpissimo/target/sim/questasim/Makefile .


git submodule update --init –recursive

Add this toolchain : 

riscv-gnu-toolchain/2024.12.16

pulp-runtime/configs/pulp.tcsh

setenv PULP_RISCV_GCC_TOOLCHAIN /sw/freetools/riscv-gnu-toolchain/2024.12.16/rh80_64

setenv PULPRT_HOME /prj/adgsys/users/prateek/pulp_12nov/pulpissimo/sw/pulp-runtime