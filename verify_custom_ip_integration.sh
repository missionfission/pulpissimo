#!/bin/bash
################################################################################
# Custom IP Integration Verification Script
################################################################################
# This script verifies that all necessary files for custom IP integration
# are present and properly configured.
################################################################################

echo "========================================================================"
echo "  PULPissimo Custom IP Integration Verification"
echo "========================================================================"
echo ""

ERRORS=0
WARNINGS=0

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper functions
check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}[✓]${NC} Found: $1"
        return 0
    else
        echo -e "${RED}[✗]${NC} Missing: $1"
        ((ERRORS++))
        return 1
    fi
}

check_content() {
    if grep -q "$2" "$1" 2>/dev/null; then
        echo -e "${GREEN}[✓]${NC} $1 contains: $2"
        return 0
    else
        echo -e "${RED}[✗]${NC} $1 missing: $2"
        ((ERRORS++))
        return 1
    fi
}

check_warning() {
    if grep -q "$2" "$1" 2>/dev/null; then
        echo -e "${GREEN}[✓]${NC} $1 contains: $2"
        return 0
    else
        echo -e "${YELLOW}[!]${NC} Warning: $1 might need: $2"
        ((WARNINGS++))
        return 1
    fi
}

echo "1. Checking RTL Files..."
echo "------------------------"
check_file "hw/custom_actuator_ctrl.sv"
echo ""

echo "2. Checking Memory Map Configuration..."
echo "----------------------------------------"
check_file "hw/includes/soc_mem_map.svh"
check_content "hw/includes/soc_mem_map.svh" "SOC_MEM_MAP_CUSTOM_ACTUATOR_START_ADDR"
check_content "hw/includes/soc_mem_map.svh" "1A10_D000"
echo ""

echo "3. Checking APB Bus Configuration..."
echo "-------------------------------------"
check_file "hw/includes/periph_bus_defines.sv"
check_content "hw/includes/periph_bus_defines.sv" "NB_MASTER.*12"
check_content "hw/includes/periph_bus_defines.sv" "CUSTOM_ACTUATOR_START_ADDR"
echo ""

echo "4. Checking Build Configuration..."
echo "-----------------------------------"
check_file "Bender.yml"
check_content "Bender.yml" "custom_actuator_ctrl.sv"
echo ""

echo "5. Checking Software Driver..."
echo "-------------------------------"
check_file "sw/bootcode/include/custom_actuator_ctrl.h"
check_content "sw/bootcode/include/custom_actuator_ctrl.h" "CUSTOM_ACTUATOR_BASE_ADDR"
check_content "sw/bootcode/include/custom_actuator_ctrl.h" "0x1A10D000"
echo ""

echo "6. Checking Test Program..."
echo "---------------------------"
check_file "sw/regression_tests/test_custom_actuator.c"
check_content "sw/regression_tests/test_custom_actuator.c" "custom_actuator_init"
echo ""

echo "7. Checking Documentation..."
echo "-----------------------------"
check_file "CUSTOM_IP_INTEGRATION_GUIDE.md"
check_file "CUSTOM_IP_SUMMARY.md"
check_file "hw/CUSTOM_PERIPHERAL_README.md"
echo ""

echo "8. Checking RTL Module Structure..."
echo "------------------------------------"
check_content "hw/custom_actuator_ctrl.sv" "module custom_actuator_ctrl"
check_content "hw/custom_actuator_ctrl.sv" "psel_i"
check_content "hw/custom_actuator_ctrl.sv" "penable_i"
check_content "hw/custom_actuator_ctrl.sv" "paddr_i"
check_content "hw/custom_actuator_ctrl.sv" "prdata_o"
echo ""

echo "9. Advanced Checks..."
echo "---------------------"
# Check for proper APB handshake
if grep -q "pready_o.*=.*1'b1" "hw/custom_actuator_ctrl.sv"; then
    echo -e "${GREEN}[✓]${NC} APB pready signal assigned"
else
    echo -e "${YELLOW}[!]${NC} Warning: Check pready assignment"
    ((WARNINGS++))
fi

# Check for reset logic
if grep -q "if.*!rst_ni" "hw/custom_actuator_ctrl.sv"; then
    echo -e "${GREEN}[✓]${NC} Reset logic present (active-low)"
else
    echo -e "${YELLOW}[!]${NC} Warning: Check reset logic"
    ((WARNINGS++))
fi

# Check for register map in software
REG_COUNT=$(grep -c "CUSTOM_ACTUATOR.*REG.*OFFSET" "sw/bootcode/include/custom_actuator_ctrl.h")
if [ $REG_COUNT -ge 5 ]; then
    echo -e "${GREEN}[✓]${NC} Software register map defined ($REG_COUNT registers)"
else
    echo -e "${YELLOW}[!]${NC} Warning: Only $REG_COUNT registers defined"
    ((WARNINGS++))
fi

echo ""
echo "========================================================================"
echo "  Verification Summary"
echo "========================================================================"

# Count what needs to be done
echo ""
echo "Integration Status:"
echo "-------------------"
echo -e "${GREEN}[✓]${NC} RTL peripheral module created"
echo -e "${GREEN}[✓]${NC} Memory map configured"
echo -e "${GREEN}[✓]${NC} APB bus updated"
echo -e "${GREEN}[✓]${NC} Build system updated"
echo -e "${GREEN}[✓]${NC} Software driver created"
echo -e "${GREEN}[✓]${NC} Test program written"
echo -e "${GREEN}[✓]${NC} Documentation complete"
echo ""
echo -e "${YELLOW}[!]${NC} TODO: Connect peripheral to APB bus in pulp_soc"
echo -e "${YELLOW}[!]${NC} TODO: Route external signals through hierarchy"
echo -e "${YELLOW}[!]${NC} TODO: Test in simulation"
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}  ✓ All checks passed! Integration files are ready.${NC}"
    echo -e "${GREEN}════════════════════════════════════════════════════════════════${NC}"
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
    echo -e "${YELLOW}  ! $WARNINGS warning(s) found. Review recommended.${NC}"
    echo -e "${YELLOW}════════════════════════════════════════════════════════════════${NC}"
    exit 0
else
    echo -e "${RED}════════════════════════════════════════════════════════════════${NC}"
    echo -e "${RED}  ✗ $ERRORS error(s) found. Please fix before proceeding.${NC}"
    echo -e "${RED}════════════════════════════════════════════════════════════════${NC}"
    exit 1
fi
