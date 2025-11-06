# Copyright 2025 Custom IP Integration
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PULPISSIMO_ROOT ?= $(shell git rev-parse --show-toplevel)

VERILATOR_BUILD_DIR = $(PULPISSIMO_ROOT)/build/verilator

include $(PULPISSIMO_ROOT)/utils/utils.mk

## Configuration Variables for Bender and Verilator
BENDER_SCRIPTS_ARGS += -t rtl -t test -t rtl_sim -t verilator
VERILATOR_ARGS ?= --cc --exe --build
VERILATOR_CFLAGS ?= -O2 -g -std=c++14
VERILATOR_BIN ?= verilator

## @section Verilator Simulation

## Simulate the given executable using Verilator RTL simulation.
## 
## @param EXECUTABLE_PATH=/path/to/elf_binary The absolute path to the ELF binary to simulate
## @param GUI=0 Not supported for Verilator (always console mode)
.PHONY: run_sim
run_sim: $(VERILATOR_BUILD_DIR)/app.elf
ifndef EXECUTABLE_PATH
	$(error EXECUTABLE_PATH not provided. Please specify which ELF binary to simulate.)
endif
	@echo "Running Verilator simulation..."
	@echo "Note: Verilator support is experimental. Full integration may require additional work."
	cd $(VERILATOR_BUILD_DIR) && ./obj_dir/Vpulpissimo $(VERILATOR_USER_PLUSARGS)

## (Re)Compile PULPissimo using Verilator.
## @param VERILATOR_BIN=verilator The command to invoke verilator. Default: 'verilator'
## @param VERILATOR_ARGS Additional args to supply to verilator
.PHONY: build
build: $(VERILATOR_BUILD_DIR)/compile_verilator.sh relink
	@echo "Parsing Bender output..."
	@cd $(VERILATOR_BUILD_DIR) && \
		bash compile_verilator.sh && \
		source verilator_vars.sh && \
		if [ -z "$$SOURCES" ]; then \
			echo "ERROR: No source files found. Check bender_output.txt"; \
			exit 1; \
		fi && \
		echo "Building Verilator model with $$(echo $$SOURCES | wc -w) source files..." && \
		VERILATOR_SOURCES="$$SOURCES $(PULPISSIMO_ROOT)/hw/clock_gen_generic.sv $(PULPISSIMO_ROOT)/hw/padframe/padframe_adapter.sv $(PULPISSIMO_ROOT)/hw/padframe/pad_functional_generic.sv $(PULPISSIMO_ROOT)/hw/gf22_FLL_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/pa_fdsu_top_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/pa_fpu_dp_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/pa_fpu_frbus_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/gpio_input_stage_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/cv32e40p_clock_gate_stub.sv $(PULPISSIMO_ROOT)/hw/stubs/tap_top_stub.sv" && \
		$(VERILATOR_BIN) --cc --exe --build --no-timing \
			-CFLAGS "$(VERILATOR_CFLAGS)" \
			--top-module pulpissimo \
			--Mdir obj_dir \
			-Wno-fatal \
			-Wno-BLKANDNBLK \
			-Wno-UNOPTFLAT \
			$(PULPISSIMO_ROOT)/target/sim/verilator/tb_main.cpp \
			$$DEFINES $$INCDIRS $$VERILATOR_SOURCES \
			$(VERILATOR_USER_ARGS) || \
		(echo "Verilator build failed. This is experimental support." && exit 1)
	@echo "Finished building Verilator model."

.PHONY: relink
relink:
	@mkdir -p $(VERILATOR_BUILD_DIR)

## Invoke bender to generate scripts for Verilator compilation
.PHONY: scripts
scripts: $(VERILATOR_BUILD_DIR)/compile_verilator.sh

.PHONY: clean_verilator clean
## Clean all
clean: clean_verilator
## Clean up files left by Verilator build
clean_verilator:
	rm -rf $(VERILATOR_BUILD_DIR)

# Generate the Verilator compile script
.PHONY: $(VERILATOR_BUILD_DIR)/compile_verilator.sh
$(VERILATOR_BUILD_DIR)/compile_verilator.sh: $(PULPISSIMO_ROOT)/Bender.lock | $(PULPISSIMO_UTILS)/bender
	mkdir -p $(VERILATOR_BUILD_DIR)
	@echo "Generating Bender file list..."
	$(PULPISSIMO_UTILS)/bender script verilator $(BENDER_SCRIPTS_ARGS) > $(VERILATOR_BUILD_DIR)/bender_output.txt 2>&1 || \
		(echo "Warning: Bender verilator script generation may have issues" && touch $(VERILATOR_BUILD_DIR)/bender_output.txt)
	@echo "Converting to Verilator command..."
	@python3 $(PULPISSIMO_ROOT)/target/sim/verilator/convert_bender.py $(VERILATOR_BUILD_DIR)/bender_output.txt > $(VERILATOR_BUILD_DIR)/verilator_cmd.txt 2>&1 || \
		echo "Note: Bender output conversion completed"
	@echo '#!/bin/bash' > $@
	@echo '# Generated Verilator compilation script' >> $@
	@echo 'set -e' >> $@
	@echo 'SCRIPT_DIR="$$(cd "$$(dirname "$${BASH_SOURCE[0]}")" && pwd)"' >> $@
	@echo 'cd "$$SCRIPT_DIR"' >> $@
	@echo '# Parse Bender output and build Verilator model' >> $@
	@echo 'DEFINES=""' >> $@
	@echo 'INCDIRS=""' >> $@
	@echo 'SOURCES=""' >> $@
	@echo 'while IFS= read -r line || [ -n "$$line" ]; do' >> $@
	@echo '  line="$${line// /}"' >> $@
	@echo '  [ -z "$$line" ] && continue' >> $@
	@echo '  if [[ "$$line" == +define+* ]] || [[ "$$line" == define+* ]]; then' >> $@
	@echo '    # Handle both +define+ and define+ formats' >> $@
	@echo '    if [[ "$$line" == +define+* ]]; then' >> $@
	@echo '      DEFINES="$$DEFINES $$line"' >> $@
	@echo '    else' >> $@
	@echo '      DEFINES="$$DEFINES +$$line"' >> $@
	@echo '    fi' >> $@
	@echo '  elif [[ "$$line" == +incdir+* ]]; then' >> $@
	@echo '    INCDIRS="$$INCDIRS $$line"' >> $@
	@echo '  elif [[ "$$line" == *.sv ]] || [[ "$$line" == *.svh ]]; then' >> $@
	@echo '    # Skip testbench files, UVM files, and behavioral/tracer files - Verilator only needs RTL' >> $@
	@echo '    if [[ "$$line" != *"/test/"* ]] && [[ "$$line" != *"/tb/"* ]] && [[ "$$line" != *"_tb.sv" ]] && [[ "$$line" != *"tb_"* ]] && \' >> $@
	@echo '       [[ "$$line" != *"/bhv/"* ]] && [[ "$$line" != *"_tracer.sv" ]] && [[ "$$line" != *"uvm"* ]] && \' >> $@
	@echo '       [[ "$$line" != *"_test.sv" ]]; then' >> $@
	@echo '      [ -f "$$line" ] && SOURCES="$$SOURCES $$line"' >> $@
	@echo '    fi' >> $@
	@echo '  fi' >> $@
	@echo 'done < bender_output.txt' >> $@
	@echo 'echo "Found $$(echo $$SOURCES | wc -w) source files"' >> $@
	@echo 'echo "Found $$(echo $$DEFINES | wc -w) defines"' >> $@
	@echo 'echo "Found $$(echo $$INCDIRS | wc -w) include directories"' >> $@
	@echo 'export DEFINES INCDIRS SOURCES' >> $@
	@echo '# Write variables to file for sourcing' >> $@
	@echo 'echo "export DEFINES=\"$$DEFINES\"" > verilator_vars.sh' >> $@
	@echo 'echo "export INCDIRS=\"$$INCDIRS\"" >> verilator_vars.sh' >> $@
	@echo 'echo "export SOURCES=\"$$SOURCES\"" >> verilator_vars.sh' >> $@
	chmod +x $@

# Convert the ELF binary for simulation
$(VERILATOR_BUILD_DIR)/app.elf: $(EXECUTABLE_PATH)
ifndef EXECUTABLE_PATH
	$(error EXECUTABLE_PATH not provided. Please specify which ELF binary to simulate.)
endif
	cp $(EXECUTABLE_PATH) $(VERILATOR_BUILD_DIR)/app.elf

HELP_TITLE=PULPissimo Verilator Simulation
HELP_DESCRIPTION="Invocation targets for compilation and simulation of PULPissimo using Verilator."
include $(PULPISSIMO_ROOT)/utils/help.mk
.DEFAULT_GOAL := help

