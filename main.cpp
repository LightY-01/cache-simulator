#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

// Include the cache policy headers
#include "src/policies/cache-policy.h"
#include "src/policies/lru-cache.h"
#include "src/policies/lfu-cache.h"
#include "src/policies/arc-cache.h"

using namespace std;

// Struct to read the packed binary trace format
#pragma pack(push, 1)
struct Trace {
    uint64_t address;
    uint32_t size;
    bool is_read;
};
#pragma pack(pop)

// Generic simulation runner that works for ANY cache policy using polymorphism
void run_cache_simulation(CachePolicy* policy, const vector<Trace>& traces, int cache_capacity) {
    policy->clear(); // Ensure cache starts clean
    
    int total_blocks_accessed = 0;
    int hits = 0;
    int misses = 0;
    
    // Loop through each request and simulate block-level accesses
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
            
            // Try to lookup key in the cache
            if (policy->get(block_addr) != -1) {
                hits++; // Cache Hit
            } else {
                misses++; // Cache Miss
                policy->put(block_addr, 1); // Insert into cache
            }
        }
    }
    
    double hit_rate = ((double)hits / total_blocks_accessed) * 100.0;
    
    // Print out the metrics
    cout << "Policy Name:          " << policy->get_name() << endl;
    cout << "  Cache Capacity:     " << cache_capacity << " blocks" << endl;
    cout << "  Total Accesses:     " << total_blocks_accessed << endl;
    cout << "  Cache Hits:         " << hits << endl;
    cout << "  Cache Misses:       " << misses << endl;
    cout << "  Cache Hit Rate:     " << hit_rate << "%" << endl;
    cout << "-------------------------------------" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "To run trace simulation, pass the binary trace path." << endl;
        cout << "Example: " << argv[0] << " Financial1.bin" << endl;
        return 1;
    }
    
    string trace_path = argv[1];
    cout << "Opening binary trace: " << trace_path << endl;
    
    ifstream fin(trace_path, ios::binary);
    if (!fin) {
        cerr << "Error: Could not open trace file: " << trace_path << endl;
        return 1;
    }
    
    // Load trace file into memory
    vector<Trace> traces;
    Trace trace;
    while (fin.read(reinterpret_cast<char*>(&trace), sizeof(Trace))) {
        traces.push_back(trace);
    }
    
    cout << "Loaded " << traces.size() << " requests." << endl;
    cout << "Simulating cache policies..." << endl;
    cout << "-------------------------------------" << endl;
    
    // Define the cache capacity (e.g. 50,000 blocks)
    int cache_capacity = 50000;
    
    // Initialize polymorphic policies list
    vector<CachePolicy*> policies = {
        new LRUCache(cache_capacity),
        new LFUCache(cache_capacity),
        new ARCCache(cache_capacity)
    };
    
    // Run simulation for each policy
    for (CachePolicy* policy : policies) {
        run_cache_simulation(policy, traces, cache_capacity);
        delete policy; // Free memory to avoid leaks
    }
    
    return 0;
}
