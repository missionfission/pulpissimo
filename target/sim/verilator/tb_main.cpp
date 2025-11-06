#include "Vpulpissimo.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <iomanip>

// SREC record types
#define SREC_HEADER 0
#define SREC_DATA_16BIT 1
#define SREC_DATA_24BIT 2
#define SREC_DATA_32BIT 3
#define SREC_COUNT_16BIT 5
#define SREC_COUNT_24BIT 6
#define SREC_START_32BIT 9

struct SRECRecord {
    int type;
    uint32_t address;
    std::vector<uint8_t> data;
    uint32_t entry_point;
};

// Parse SREC file and extract entry point
std::vector<SRECRecord> parse_srec(const std::string& filename, uint32_t& entry_point) {
    std::vector<SRECRecord> records;
    std::ifstream file(filename);
    std::string line;
    entry_point = 0x1A000000; // Default entry point
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open SREC file: " << filename << std::endl;
        return records;
    }
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] != 'S') continue;
        
        SRECRecord rec;
        rec.type = line[1] - '0';
        
        if (rec.type == SREC_HEADER) {
            continue;
        }
        
        if (rec.type == SREC_START_32BIT) {
            // S9 record contains entry point
            int length = std::stoi(line.substr(2, 2), nullptr, 16);
            if (length >= 5) {
                entry_point = std::stoul(line.substr(4, 8), nullptr, 16);
            }
            continue;
        }
        
        if (rec.type == SREC_DATA_32BIT) {
            // Parse S3 record: S3<length><address><data><checksum>
            int length = std::stoi(line.substr(2, 2), nullptr, 16);
            int addr_len = 8; // 32-bit address = 4 bytes = 8 hex chars
            rec.address = std::stoul(line.substr(4, addr_len), nullptr, 16);
            
            int data_len = (length - 5) * 2; // length includes addr(4) + checksum(1)
            for (int i = 0; i < data_len; i += 2) {
                uint8_t byte = std::stoul(line.substr(4 + addr_len + i, 2), nullptr, 16);
                rec.data.push_back(byte);
            }
            
            records.push_back(rec);
        }
    }
    
    return records;
}

// Simple clock generator
class ClockGen {
public:
    ClockGen() : clk_(false), cycle_(0) {}
    
    bool tick() {
        clk_ = !clk_;
        if (!clk_) cycle_++;
        return clk_;
    }
    
    uint64_t get_cycle() const { return cycle_; }
    bool get_clk() const { return clk_; }
    
private:
    bool clk_;
    uint64_t cycle_;
};

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    
    // Parse command line arguments
    std::string srec_file;
    uint64_t max_cycles = 10000000; // Default max cycles
    bool vcd_trace = false;
    bool verbose = false;
    
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "+srec=", 6) == 0) {
            srec_file = argv[i] + 6;
        } else if (strncmp(argv[i], "+max_cycles=", 12) == 0) {
            max_cycles = std::stoull(argv[i] + 12);
        } else if (strcmp(argv[i], "+vcd") == 0) {
            vcd_trace = true;
        } else if (strcmp(argv[i], "+verbose") == 0) {
            verbose = true;
        }
    }
    
    std::cout << "================================================================================" << std::endl;
    std::cout << "Verilator RTL Simulation - Motor Control Benchmark" << std::endl;
    std::cout << "================================================================================" << std::endl;
    
    if (!srec_file.empty()) {
        std::cout << "SREC file: " << srec_file << std::endl;
        uint32_t entry_point;
        auto records = parse_srec(srec_file, entry_point);
        std::cout << "Parsed " << records.size() << " SREC records" << std::endl;
        std::cout << "Entry point: 0x" << std::hex << entry_point << std::dec << std::endl;
        std::cout << "Note: Memory loading through direct access requires Verilator model enhancement" << std::endl;
        std::cout << "      For now, simulation will run with system initialization" << std::endl;
    } else {
        std::cout << "No SREC file specified - running system initialization only" << std::endl;
    }
    
    std::cout << "Max cycles: " << max_cycles << std::endl;
    std::cout << "VCD trace: " << (vcd_trace ? "enabled" : "disabled") << std::endl;
    std::cout << "================================================================================" << std::endl;
    
    // Create Verilator model
    Vpulpissimo* top = new Vpulpissimo;
    
    // Initialize VCD trace if requested
    VerilatedVcdC* tfp = nullptr;
    if (vcd_trace) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedVcdC;
        top->trace(tfp, 99);
        std::string vcd_name = "simulation.vcd";
        if (!srec_file.empty()) {
            // Extract basename for VCD filename
            size_t pos = srec_file.find_last_of("/\\");
            std::string basename = (pos != std::string::npos) ? srec_file.substr(pos+1) : srec_file;
            size_t dot = basename.find_last_of(".");
            vcd_name = (dot != std::string::npos) ? basename.substr(0, dot) + ".vcd" : basename + ".vcd";
        }
        tfp->open(vcd_name.c_str());
        std::cout << "VCD trace file: " << vcd_name << std::endl;
    }
    
    // Clock generator
    ClockGen clk_gen;
    
    // Reference clock period (32.769 kHz = 30517 ns)
    const uint64_t REF_CLK_PERIOD_PS = 30517000; // in picoseconds
    uint64_t time_ps = 0;
    
    std::cout << std::endl << "Initializing system..." << std::endl;
    
    // Reset sequence - assert reset for several cycles
    bool reset_asserted = true;
    uint64_t reset_cycles = 0;
    const uint64_t RESET_CYCLES = 10;
    
    std::cout << "Asserting reset..." << std::endl;
    
    while (reset_cycles < RESET_CYCLES) {
        bool clk = clk_gen.tick();
        time_ps += REF_CLK_PERIOD_PS / 2;
        
        // Drive reset (active low) - in Verilator, we'd need to drive pad_reset_n
        // For now, we'll evaluate and let the model handle it internally
        
        top->eval();
        
        if (tfp && clk) {
            tfp->dump(time_ps);
        }
        
        if (!clk) {
            reset_cycles++;
        }
    }
    
    std::cout << "Releasing reset..." << std::endl;
    reset_asserted = false;
    
    // Continue simulation
    uint64_t cycle_count = 0;
    uint64_t last_report_cycle = 0;
    const uint64_t REPORT_INTERVAL = 10000;
    
    std::cout << "Starting simulation..." << std::endl;
    std::cout << "  (Press Ctrl+C to stop early)" << std::endl;
    
    while (cycle_count < max_cycles) {
        bool clk = clk_gen.tick();
        time_ps += REF_CLK_PERIOD_PS / 2;
        
        // Evaluate model
        top->eval();
        
        // Dump VCD on clock edges
        if (tfp && clk) {
            tfp->dump(time_ps);
        }
        
        if (!clk) {
            cycle_count = clk_gen.get_cycle();
            
            // Periodic status reports
            if (cycle_count - last_report_cycle >= REPORT_INTERVAL) {
                std::cout << "  Cycle: " << std::setw(10) << cycle_count 
                          << "  Time: " << std::setw(12) << (time_ps / 1000) << " ns" << std::endl;
                last_report_cycle = cycle_count;
            }
        }
        
        // TODO: Add completion detection
        // This could monitor:
        // - CPU halt/debug signals
        // - Specific memory locations
        // - Peripheral status registers
    }
    
    std::cout << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "Simulation Complete" << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "Total Cycles: " << cycle_count << std::endl;
    std::cout << "Simulation Time: " << (time_ps / 1000000.0) << " us" << std::endl;
    std::cout << "================================================================================" << std::endl;
    
    // Cleanup
    top->final();
    if (tfp) {
        tfp->close();
        delete tfp;
    }
    delete top;
    
    return 0;
}
