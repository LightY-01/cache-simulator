#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

// Include the cache policy files
#include "src/policies/lru-cache.h"
#include "src/policies/lfu-cache.h"

using namespace std;

// Struct to read the packed binary trace format
#pragma pack(push, 1)
struct Trace {
    uint64_t address;
    uint32_t size;
    bool is_read;
};
#pragma pack(pop)

// 1. ADVERSARIAL TEST FOR LRU: Sequential Thrashing Scan
// Access loop of size N = cache_size + 1. Both LRU and LFU will thrash (0% hit rate).
void run_lru_thrashing_test() {
    cout << "=== Test 1: LRU Adversarial (Sequential Thrashing) ===" << endl;
    
    int cache_size = 10;
    int N = cache_size + 1; // 11 unique blocks (0 to 10)
    int total_accesses = 100;
    
    LRUCache lru(cache_size);
    LFUCache lfu(cache_size);
    
    int lru_hits = 0;
    int lfu_hits = 0;
    
    // Access sequence: 0, 1, 2, ..., 10, 0, 1, 2, ..., 10, ...
    for (int i = 0; i < total_accesses; i++) {
        uint64_t block_addr = i % N;
        
        // Test LRU
        if (lru.get(block_addr) != -1) {
            lru_hits++;
        } else {
            lru.put(block_addr, 1);
        }
        
        // Test LFU
        if (lfu.get(block_addr) != -1) {
            lfu_hits++;
        } else {
            lfu.put(block_addr, 1);
        }
    }
    
    cout << "Cache Size:          " << cache_size << " blocks" << endl;
    cout << "Access Sequence:     0 to " << N-1 << " repeatedly" << endl;
    cout << "LRU Hit Rate:        " << ((double)lru_hits / total_accesses) * 100.0 << "%" << endl;
    cout << "LFU Hit Rate:        " << ((double)lfu_hits / total_accesses) * 100.0 << "%" << endl << endl;
}

// 2. ADVERSARIAL TEST FOR LFU: Frequency Shock / Cache Pollution
// We access blocks 1-10 many times (making their frequency 100).
// Then we switch to blocks 11-20. LFU stays at 0% hit rate because it refuses to evict
// the stale high-frequency blocks. LRU adapts quickly and gets a high hit rate.
void run_lfu_frequency_shock_test() {
    cout << "=== Test 2: LFU Adversarial (Frequency Shock / Cache Pollution) ===" << endl;
    
    int cache_size = 10;
    LRUCache lru(cache_size);
    LFUCache lfu(cache_size);
    
    // Phase 1: Access blocks 1 to 10 heavily to build up frequency = 100
    int burst_count = 100;
    for (int f = 0; f < burst_count; f++) {
        for (uint64_t block = 1; block <= 10; block++) {
            lru.put(block, 1);
            lfu.put(block, 1);
        }
    }
    
    // Phase 2: Shift working set completely to blocks 11 to 20
    int test_accesses = 100;
    int lru_hits = 0;
    int lfu_hits = 0;
    
    for (int i = 0; i < test_accesses; i++) {
        uint64_t block_addr = 11 + (i % 10);
        
        // Test LRU
        if (lru.get(block_addr) != -1) {
            lru_hits++;
        } else {
            lru.put(block_addr, 1);
        }
        
        // Test LFU
        if (lfu.get(block_addr) != -1) {
            lfu_hits++;
        } else {
            lfu.put(block_addr, 1);
        }
    }
    
    cout << "Cache Size:          " << cache_size << " blocks" << endl;
    cout << "Phase 1: Loaded blocks 1-10 with frequency = " << burst_count << endl;
    cout << "Phase 2: Shifted working set to blocks 11-20 for " << test_accesses << " accesses" << endl;
    cout << "LRU Hit Rate (P2):   " << ((double)lru_hits / test_accesses) * 100.0 << "%" << endl;
    cout << "LFU Hit Rate (P2):   " << ((double)lfu_hits / test_accesses) * 100.0 << "%" << endl << endl;
}

// 3. REAL-WORLD TRACE SIMULATION
void run_trace_simulation(string trace_path) {
    cout << "=== Test 3: Running Trace File Simulation ===" << endl;
    cout << "Opening binary trace: " << trace_path << endl;
    
    ifstream fin(trace_path, ios::binary);
    if (!fin) {
        cerr << "Error: Could not open trace file: " << trace_path << endl;
        return;
    }
    
    int cache_capacity = 50000;
    LRUCache lru(cache_capacity);
    LFUCache lfu(cache_capacity);
    
    vector<Trace> traces;
    Trace trace;
    
    // Read the binary file into memory
    while (fin.read(reinterpret_cast<char*>(&trace), sizeof(Trace))) {
        traces.push_back(trace);
    }
    
    cout << "Loaded " << traces.size() << " requests." << endl;
    cout << "Simulating cache policies..." << endl;
    
    int total_blocks_accessed = 0;
    int lru_hits = 0;
    int lfu_hits = 0;
    
    // Loop through each request and simulate accesses block-by-block
    for (int i = 0; i < traces.size(); i++) {
        Trace req = traces[i];
        
        // Calculate number of 512-byte blocks accessed
        int blocks_count = (req.size + 511) / 512;
        if (blocks_count == 0) {
            blocks_count = 1;
        }
        
        for (int b = 0; b < blocks_count; b++) {
            uint64_t block_addr = req.address + b;
            total_blocks_accessed++;
            
            // Simulate LRU
            if (lru.get(block_addr) != -1) {
                lru_hits++;
            } else {
                lru.put(block_addr, 1);
            }
            
            // Simulate LFU
            if (lfu.get(block_addr) != -1) {
                lfu_hits++;
            } else {
                lfu.put(block_addr, 1);
            }
        }
    }
    
    double lru_hit_rate = ((double)lru_hits / total_blocks_accessed) * 100.0;
    double lfu_hit_rate = ((double)lfu_hits / total_blocks_accessed) * 100.0;
    
    cout << "Cache Capacity:       " << cache_capacity << " blocks" << endl;
    cout << "Total Block Accesses: " << total_blocks_accessed << endl;
    cout << "LRU Hits:             " << lru_hits << " (Hit Rate: " << lru_hit_rate << "%)" << endl;
    cout << "LFU Hits:             " << lfu_hits << " (Hit Rate: " << lfu_hit_rate << "%)" << endl;
}

int main(int argc, char* argv[]) {
    // 1. Run the thrashing adversarial test (targets LRU limitations)
    run_lru_thrashing_test();
    
    // 2. Run the frequency shock adversarial test (targets LFU limitations)
    run_lfu_frequency_shock_test();
    
    // 3. Run the disk trace simulation if trace path is provided
    if (argc >= 2) {
        string trace_path = argv[1];
        run_trace_simulation(trace_path);
    } else {
        cout << "To run trace simulation, pass the binary trace path." << endl;
        cout << "Example: " << argv[0] << " Financial1.bin" << endl;
    }
    
    return 0;
}
