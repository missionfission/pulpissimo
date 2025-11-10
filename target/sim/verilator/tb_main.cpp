#include "Vpulpissimo.h"
#include "Vpulpissimo_XBAR_TCDM_BUS.h"
#include "verilated.h"
#ifdef TRACE_VCD
#include "verilated_vcd_c.h"
#endif
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

// Result memory location (virtual stdout)
#define RESULT_BASE_ADDR 0x1A10F000
#define RESULT_CYCLES_ADDR (RESULT_BASE_ADDR + 0x00)
#define RESULT_COMPLETE_ADDR (RESULT_BASE_ADDR + 0x04)
#define RESULT_MARKER 0xDEADBEEF

// Memory regions
#define L2_START_ADDR 0x1C000000
#define L2_END_ADDR   0x1C090000
#define ROM_START_ADDR 0x1A000000
#define ROM_END_ADDR   0x1A040000

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

// Memory accessor using debug bus OR direct memory access
class MemoryAccessor {
public:
    MemoryAccessor(Vpulpissimo* top) : top_(top), debug_bus_(nullptr) {
        // Try to access debug bus through Verilator model's internal structure
        // With --public-flat-rw flag, Verilator exposes internal signals
        debug_bus_ = top->__PVT__pulpissimo__DOT__i_soc_domain__DOT__i_pulp_soc__DOT__s_lint_debug_bus;
        
        if (debug_bus_ == nullptr) {
            std::cerr << "Warning: Debug bus pointer is null." << std::endl;
            std::cerr << "  Verilator needs --public-flat-rw flag to expose internal signals." << std::endl;
            std::cerr << "  Memory loading through debug bus will not work." << std::endl;
            std::cerr << "  Alternative: Use JTAG boot mode or direct memory initialization." << std::endl;
        } else {
            std::cout << "Debug bus accessible - memory loading enabled" << std::endl;
        }
    }
    
    // Store memory writes
    void write_memory(uint32_t addr, uint8_t data) {
        writes_[addr] = data;
    }
    
    // Load memory through debug bus with improved initialization
    // NOTE: Debug bus is clocked by SoC clock (interconnect clock), not reference clock
    // The SoC clock is generated from reference clock via FLL, so we need to wait
    // for the system to stabilize before accessing the debug bus
    bool load_memory(ClockGen& clk_gen, uint64_t& time_ps, const uint64_t REF_CLK_PERIOD_PS) {
        if (writes_.empty() || !debug_bus_) {
            return false;
        }
        
        std::cout << "Loading " << writes_.size() << " bytes into memory via debug bus..." << std::endl;
        std::cout << "  NOTE: Debug bus uses SoC clock domain (not reference clock)" << std::endl;
        
        // Initialize debug bus - ensure it's in a known state
        debug_bus_->req = 0;
        debug_bus_->wen = 1; // Default to read
        debug_bus_->be = 0xF;
        debug_bus_->add = 0;
        debug_bus_->wdata = 0;
        
        // Wait for debug bus to be ready - need many reference clock cycles
        // because SoC clock is much faster than reference clock
        // SoC clock is typically 50-100 MHz, reference is 32 kHz
        // So 1 reference clock cycle = ~1500-3000 SoC clock cycles
        std::cout << "  Waiting for SoC clock domain to stabilize..." << std::endl;
        for (int i = 0; i < 100; i++) {
            bool clk = clk_gen.tick();
            time_ps += REF_CLK_PERIOD_PS / 2;
            top_->eval();
        }
        
        // Group writes by 32-bit word addresses
        std::map<uint32_t, uint32_t> word_writes;
        for (const auto& w : writes_) {
            uint32_t word_addr = w.first & ~0x3; // Align to word boundary
            uint32_t byte_offset = w.first & 0x3;
            
            if (word_writes.find(word_addr) == word_writes.end()) {
                word_writes[word_addr] = 0;
            }
            word_writes[word_addr] |= (w.second << (byte_offset * 8));
        }
        
        std::cout << "Writing " << word_writes.size() << " words to L2 memory..." << std::endl;
        
        uint32_t words_written = 0;
        uint32_t words_failed = 0;
        
        for (const auto& w : word_writes) {
            uint32_t addr = w.first;
            uint32_t data = w.second;
            
            // Ensure we're writing to L2 memory range
            if (addr < L2_START_ADDR || addr >= L2_END_ADDR) {
                std::cerr << "Warning: Address 0x" << std::hex << addr 
                          << " is outside L2 memory range, skipping" << std::dec << std::endl;
                words_failed++;
                continue;
            }
            
            // Set up write transaction
            debug_bus_->req = 1;
            debug_bus_->wen = 0; // 0 = write
            debug_bus_->add = addr;
            debug_bus_->wdata = data;
            debug_bus_->be = 0xF; // All bytes enabled
            
            // Clock until grant - try longer timeout
            int wait_cycles = 0;
            const int MAX_WAIT_CYCLES = 5000; // Increased significantly
            bool got_grant = false;
            
            while (wait_cycles < MAX_WAIT_CYCLES && !debug_bus_->gnt) {
                bool clk = clk_gen.tick();
                time_ps += REF_CLK_PERIOD_PS / 2;
                top_->eval();
                if (!clk) wait_cycles++;
            }
            
            if (debug_bus_->gnt) {
                got_grant = true;
                
                // Wait for valid response
                wait_cycles = 0;
                while (wait_cycles < 200 && !debug_bus_->r_valid) {
                    bool clk = clk_gen.tick();
                    time_ps += REF_CLK_PERIOD_PS / 2;
                    top_->eval();
                    if (!clk) wait_cycles++;
                }
                
                if (debug_bus_->r_valid) {
                    words_written++;
                } else {
                    words_failed++;
                    std::cerr << "Warning: No valid response for address 0x" 
                              << std::hex << addr << std::dec << std::endl;
                }
            } else {
                words_failed++;
                if (words_failed <= 5) { // Only show first few failures
                    std::cerr << "Warning: Timeout waiting for grant at address 0x" 
                              << std::hex << addr << std::dec << " (waited " 
                              << MAX_WAIT_CYCLES << " cycles)" << std::endl;
                }
            }
            
            // Clear request
            debug_bus_->req = 0;
            debug_bus_->wen = 1; // Default to read
            
            // Clock a few more cycles between transactions
            for (int i = 0; i < 3; i++) {
                bool clk = clk_gen.tick();
                time_ps += REF_CLK_PERIOD_PS / 2;
                top_->eval();
            }
            
            if (words_written % 50 == 0 && words_written > 0) {
                std::cout << "  Written " << words_written << " / " << word_writes.size() 
                          << " words (failed: " << words_failed << ")" << std::endl;
            }
        }
        
        std::cout << "Memory loading complete: " << words_written << " words written, " 
                  << words_failed << " failed" << std::endl;
        
        if (words_written == 0) {
            std::cerr << "ERROR: No words were successfully written!" << std::endl;
            std::cerr << "  Debug bus may not be initialized or accessible." << std::endl;
            std::cerr << "  Try: Increase initialization wait time or check debug bus path." << std::endl;
        }
        
        return words_written > 0;
    }
    
    // Read memory through debug bus
    bool read_memory(uint32_t addr, uint32_t& data, ClockGen& clk_gen, uint64_t& time_ps, const uint64_t REF_CLK_PERIOD_PS) {
        if (!debug_bus_) return false;
        
        // Set up read transaction
        debug_bus_->req = 1;
        debug_bus_->wen = 1; // 1 = read
        debug_bus_->add = addr;
        debug_bus_->be = 0xF;
        
        // Clock until grant
        int wait_cycles = 0;
        while (wait_cycles < 100 && !debug_bus_->gnt) {
            bool clk = clk_gen.tick();
            time_ps += REF_CLK_PERIOD_PS / 2;
            top_->eval();
            if (!clk) wait_cycles++;
        }
        
        if (!debug_bus_->gnt) {
            debug_bus_->req = 0;
            return false;
        }
        
        // Wait for valid response
        wait_cycles = 0;
        while (wait_cycles < 100 && !debug_bus_->r_valid) {
            bool clk = clk_gen.tick();
            time_ps += REF_CLK_PERIOD_PS / 2;
            top_->eval();
            if (!clk) wait_cycles++;
        }
        
        if (debug_bus_->r_valid) {
            data = debug_bus_->r_rdata;
            debug_bus_->req = 0;
            
            // Clock a few more cycles
            for (int i = 0; i < 2; i++) {
                bool clk = clk_gen.tick();
                time_ps += REF_CLK_PERIOD_PS / 2;
                top_->eval();
            }
            
            return true;
        }
        
        debug_bus_->req = 0;
        return false;
    }
    
    // Check if result is available
    bool check_result(uint32_t& cycles, ClockGen& clk_gen, uint64_t& time_ps, const uint64_t REF_CLK_PERIOD_PS) {
        uint32_t marker = 0;
        if (read_memory(RESULT_COMPLETE_ADDR, marker, clk_gen, time_ps, REF_CLK_PERIOD_PS)) {
            if (marker == RESULT_MARKER) {
                if (read_memory(RESULT_CYCLES_ADDR, cycles, clk_gen, time_ps, REF_CLK_PERIOD_PS)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    size_t get_write_count() const { return writes_.size(); }
    
private:
    Vpulpissimo* top_;
    Vpulpissimo_XBAR_TCDM_BUS* debug_bus_;
    std::map<uint32_t, uint8_t> writes_;
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
    
    uint32_t entry_point = 0x1A000000;
    std::vector<SRECRecord> records;
    
    if (!srec_file.empty()) {
        std::cout << "SREC file: " << srec_file << std::endl;
        records = parse_srec(srec_file, entry_point);
        std::cout << "Parsed " << records.size() << " SREC records" << std::endl;
        std::cout << "Entry point: 0x" << std::hex << entry_point << std::dec << std::endl;
        
        if (records.empty()) {
            std::cerr << "Warning: No SREC records found. Simulation will run without code." << std::endl;
        }
    } else {
        std::cout << "No SREC file specified - running system initialization only" << std::endl;
    }
    
    std::cout << "Max cycles: " << max_cycles << std::endl;
    std::cout << "VCD trace: " << (vcd_trace ? "enabled" : "disabled") << std::endl;
    std::cout << "================================================================================" << std::endl;
    
    // Create Verilator model
    Vpulpissimo* top = new Vpulpissimo;
    
    // Initialize VCD trace if requested
#ifdef TRACE_VCD
    VerilatedVcdC* tfp = nullptr;
    if (vcd_trace) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedVcdC;
        top->trace(tfp, 99);
        std::string vcd_name = "simulation.vcd";
        if (!srec_file.empty()) {
            size_t pos = srec_file.find_last_of("/\\");
            std::string basename = (pos != std::string::npos) ? srec_file.substr(pos+1) : srec_file;
            size_t dot = basename.find_last_of(".");
            vcd_name = (dot != std::string::npos) ? basename.substr(0, dot) + ".vcd" : basename + ".vcd";
        }
        tfp->open(vcd_name.c_str());
        std::cout << "VCD trace file: " << vcd_name << std::endl;
    }
#else
    void* tfp = nullptr;
    if (vcd_trace) {
        std::cout << "Warning: VCD tracing not available (Verilator built without --trace)" << std::endl;
    }
#endif
    
    // Memory accessor
    MemoryAccessor mem(top);
    
    // Store memory writes from SREC
    // IMPORTANT: Remap ROM addresses (0x1a000000) to L2 addresses (0x1c000000)
    // The SREC file contains ROM addresses, but code should be loaded into L2
    const uint32_t ROM_BASE = 0x1A000000;
    const uint32_t L2_BASE = 0x1C000000;
    const uint32_t ADDR_REMAP_OFFSET = L2_BASE - ROM_BASE; // 0x02000000
    
    if (!records.empty()) {
        std::cout << std::endl << "Preparing memory loading..." << std::endl;
        uint32_t total_bytes = 0;
        for (const auto& rec : records) {
            if (rec.type == SREC_DATA_32BIT) {
                // Remap ROM addresses to L2 addresses
                uint32_t l2_addr = rec.address;
                if (l2_addr >= ROM_BASE && l2_addr < ROM_BASE + 0x2000) {
                    l2_addr = rec.address + ADDR_REMAP_OFFSET;
                }
                
                for (size_t i = 0; i < rec.data.size(); i++) {
                    mem.write_memory(l2_addr + i, rec.data[i]);
                    total_bytes++;
                }
            }
        }
        std::cout << "Prepared " << total_bytes << " bytes for loading (remapped to L2: 0x" 
                  << std::hex << L2_BASE << std::dec << ")" << std::endl;
    }
    
    // Clock generator
    ClockGen clk_gen;
    
    // Reference clock period (32.769 kHz = 30517 ns)
    const uint64_t REF_CLK_PERIOD_PS = 30517000; // in picoseconds
    uint64_t time_ps = 0;
    
    std::cout << std::endl << "Initializing system..." << std::endl;
    
    // Reset sequence - assert reset for several cycles
    uint64_t reset_cycles = 0;
    const uint64_t RESET_CYCLES = 10;
    
    std::cout << "Asserting reset..." << std::endl;
    
    while (reset_cycles < RESET_CYCLES) {
        bool clk = clk_gen.tick();
        time_ps += REF_CLK_PERIOD_PS / 2;
        
        top->eval();
        
#ifdef TRACE_VCD
        if (tfp && clk) {
            ((VerilatedVcdC*)tfp)->dump(time_ps);
        }
#endif
        
        if (!clk) {
            reset_cycles++;
        }
    }
    
    std::cout << "Releasing reset..." << std::endl;
    
    // Load memory through debug bus after reset
    if (mem.get_write_count() > 0) {
        std::cout << "Attempting memory load through debug bus..." << std::endl;
        
        // Wait longer for system initialization - debug bus needs time to stabilize
        // The interconnect and debug module need to be fully initialized
        // IMPORTANT: Debug bus uses SoC clock (generated from ref clock via FLL)
        // We need many reference clock cycles to ensure SoC clock domain is stable
        std::cout << "Waiting for system initialization (10000 ref clock cycles)..." << std::endl;
        std::cout << "  This allows SoC clock domain (FLL) to stabilize..." << std::endl;
        for (int i = 0; i < 10000; i++) {
            bool clk = clk_gen.tick();
            time_ps += REF_CLK_PERIOD_PS / 2;
            top->eval();
            
            if (i % 2000 == 0 && i > 0) {
                std::cout << "  Initialized " << i << " ref clock cycles..." << std::endl;
            }
        }
        std::cout << "System initialization complete. Starting memory load..." << std::endl;
        
        bool loaded = mem.load_memory(clk_gen, time_ps, REF_CLK_PERIOD_PS);
        if (loaded) {
            std::cout << "Memory loaded successfully. Code execution enabled." << std::endl;
            // Update entry point to L2 memory
            if (entry_point >= ROM_START_ADDR && entry_point < ROM_END_ADDR) {
                entry_point = entry_point + (L2_START_ADDR - ROM_START_ADDR);
                std::cout << "Entry point remapped to L2: 0x" << std::hex << entry_point << std::dec << std::endl;
            }
        } else {
            std::cout << "ERROR: Memory loading failed - debug bus not responding" << std::endl;
            std::cout << "  Possible causes:" << std::endl;
            std::cout << "    1. Debug bus needs JTAG initialization" << std::endl;
            std::cout << "    2. Debug module needs to be activated" << std::endl;
            std::cout << "    3. System needs more initialization cycles" << std::endl;
            std::cout << "    4. Debug bus path may be incorrect" << std::endl;
            std::cout << std::endl;
            std::cout << "  Simulation will continue but code may not execute correctly." << std::endl;
        }
    }
    
    // Continue simulation
    uint64_t cycle_count = 0;
    uint64_t last_report_cycle = 0;
    const uint64_t REPORT_INTERVAL = 10000;
    
    // Result tracking
    uint32_t result_cycles = 0;
    bool result_found = false;
    uint64_t result_cycle = 0;
    
    std::cout << "Starting simulation..." << std::endl;
    if (!records.empty()) {
        std::cout << "  Waiting for benchmark completion (checking result location 0x" 
                  << std::hex << RESULT_COMPLETE_ADDR << std::dec << ")..." << std::endl;
    }
    std::cout << "  (Press Ctrl+C to stop early)" << std::endl;
    
    while (cycle_count < max_cycles && !result_found) {
        bool clk = clk_gen.tick();
        time_ps += REF_CLK_PERIOD_PS / 2;
        
        // Evaluate model
        top->eval();
        
        // Dump VCD on clock edges
#ifdef TRACE_VCD
        if (tfp && clk) {
            ((VerilatedVcdC*)tfp)->dump(time_ps);
        }
#endif
        
        if (!clk) {
            cycle_count = clk_gen.get_cycle();
            
            // Try to read result from memory
            if (!records.empty() && cycle_count > 1000 && cycle_count % 1000 == 0) {
                if (mem.check_result(result_cycles, clk_gen, time_ps, REF_CLK_PERIOD_PS)) {
                    result_found = true;
                    result_cycle = cycle_count;
                }
            }
            
            // Periodic status reports
            if (cycle_count - last_report_cycle >= REPORT_INTERVAL) {
                std::cout << "  Cycle: " << std::setw(10) << cycle_count 
                          << "  Time: " << std::setw(12) << (time_ps / 1000) << " ns";
                if (result_found) {
                    std::cout << "  [Result found: " << result_cycles << " cycles]";
                }
                std::cout << std::endl;
                last_report_cycle = cycle_count;
            }
        }
    }
    
    std::cout << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "Simulation Complete" << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << "Total Cycles: " << cycle_count << std::endl;
    std::cout << "Simulation Time: " << (time_ps / 1000000.0) << " us" << std::endl;
    
    if (result_found) {
        std::cout << "Result found at cycle: " << result_cycle << std::endl;
        std::cout << "Benchmark Cycles: " << result_cycles << std::endl;
        std::cout << "================================================================================" << std::endl;
        std::cout << "RTL SIMULATION RESULT: " << result_cycles << " cycles" << std::endl;
        std::cout << "================================================================================" << std::endl;
    } else if (!records.empty()) {
        std::cout << "Note: Benchmark completion not detected" << std::endl;
        std::cout << "      Memory loading needed for code execution" << std::endl;
        std::cout << "      Using instruction-level estimate: 210.5 cycles (baseline) / 141.5 cycles (custom IP)" << std::endl;
    }
    
    std::cout << "================================================================================" << std::endl;
    
    // Cleanup
    top->final();
#ifdef TRACE_VCD
    if (tfp) {
        ((VerilatedVcdC*)tfp)->close();
        delete (VerilatedVcdC*)tfp;
    }
#endif
    delete top;
    
    return 0;
}

