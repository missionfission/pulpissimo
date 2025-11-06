# RISC-V Toolchain Setup
# Add this to your ~/.zshrc or ~/.bashrc to make it permanent

export RISCV=$HOME/.local/riscv-pulp
export PATH=$RISCV/bin:$PATH
export PULP_RISCV_GCC_TOOLCHAIN=$RISCV

# Verify installation
if command -v riscv32-unknown-elf-gcc &> /dev/null; then
    echo "✓ RISC-V toolchain configured: $(riscv32-unknown-elf-gcc --version | head -1)"
else
    echo "✗ RISC-V toolchain not found in PATH"
fi


