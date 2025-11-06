#!/usr/bin/env python3
"""
Convert Bender Verilator script output to Verilator command-line arguments
"""
import sys
import os

def parse_bender_output(input_file):
    """Parse Bender output and generate Verilator arguments"""
    defines = []
    include_dirs = []
    source_files = []
    
    with open(input_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            # Handle defines: +define+TARGET_RTL
            if line.startswith('+define+'):
                define_name = line[8:]  # Remove '+define+'
                defines.append(f'+define+{define_name}')
            
            # Handle include directories: +incdir+/path/to/include
            elif line.startswith('+incdir+'):
                incdir = line[8:]  # Remove '+incdir+'
                include_dirs.append(f'+incdir+{incdir}')
            
            # Handle source files: /path/to/file.sv
            elif line.endswith('.sv') or line.endswith('.svh'):
                if os.path.exists(line):
                    source_files.append(line)
    
    return defines, include_dirs, source_files

def generate_verilator_command(defines, include_dirs, source_files, top_module='pulpissimo'):
    """Generate Verilator command-line"""
    cmd_parts = ['verilator']
    
    # Add defines
    cmd_parts.extend(defines)
    
    # Add include directories
    cmd_parts.extend(include_dirs)
    
    # Add source files
    cmd_parts.extend(source_files)
    
    # Add Verilator options
    cmd_parts.extend([
        '--cc',
        '--exe',
        '--build',
        f'--top-module {top_module}',
        '--Mdir obj_dir',
        '-CFLAGS "-O2 -g"'
    ])
    
    return ' '.join(cmd_parts)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: convert_bender.py <bender_output_file>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    defines, include_dirs, source_files = parse_bender_output(input_file)
    
    print(f"# Found {len(defines)} defines, {len(include_dirs)} include dirs, {len(source_files)} source files")
    print(f"# Defines: {len(defines)}")
    print(f"# Include dirs: {len(include_dirs)}")
    print(f"# Source files: {len(source_files)}")
    
    # Generate command
    cmd = generate_verilator_command(defines, include_dirs, source_files)
    print(f"\n# Verilator command:\n{cmd}")


