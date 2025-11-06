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
VERILATOR_CFLAGS ?= -O2 -g
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
	cd $(VERILATOR_BUILD_DIR) && ./Vpulpissimo $(VERILATOR_USER_PLUSARGS)

## (Re)Compile PULPissimo using Verilator.
## @param VERILATOR_BIN=verilator The command to invoke verilator. Default: 'verilator'
## @param VERILATOR_ARGS Additional args to supply to verilator
.PHONY: build
build: $(VERILATOR_BUILD_DIR)/compile_verilator.sh relink
	@echo "Generating Verilator build scripts..."
	cd $(VERILATOR_BUILD_DIR) && bash compile_verilator.sh
	@echo "Building Verilator model..."
	cd $(VERILATOR_BUILD_DIR) && $(VERILATOR_BIN) $(VERILATOR_ARGS) \
		-CFLAGS "$(VERILATOR_CFLAGS)" \
		--top-module pulpissimo \
		--Mdir obj_dir \
		$(VERILATOR_USER_ARGS)
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
	echo '#!/bin/bash' > $@
	echo '# Generated Verilator compilation script' >> $@
	echo 'set -e' >> $@
	$(PULPISSIMO_UTILS)/bender script verilator $(BENDER_SCRIPTS_ARGS) >> $@ || \
		echo "echo 'Warning: Bender verilator script generation may not be fully supported'" >> $@
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

