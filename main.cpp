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
#include "src/policies/2q-cache.h"

using namespace std;

// Struct to read the packed binary trace format
#pragma pack(push, 1)
struct Trace {
    uint64_t address;
    uint32_t size;
    bool is_read;
};
#pragma pack(pop)

// Generic simulation runner that benchmarks a cache policy
void run_cache_simulation_benchmark(CachePolicy* policy, const vector<Trace>& traces, int cache_capacity, ofstream& csv_out) {
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
    
    // Log to CSV: Policy, CacheCapacity, TotalAccesses, Hits, Misses, HitRate
    csv_out << policy->get_name() << ","
            << cache_capacity << ","
            << total_blocks_accessed << ","
            << hits << ","
            << misses << ","
            << hit_rate << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "To run benchmark, pass the binary trace path." << '\n';
        cout << "Example: " << argv[0] << " Financial1.bin" << '\n';
        return 1;
    }
    
    string trace_path = argv[1];
    cout << "Opening binary trace: " << trace_path << '\n';
    
    ifstream fin(trace_path, ios::binary);
    if (!fin) {
        cerr << "Error: Could not open trace file: " << trace_path << '\n';
        return 1;
    }
    
    // Load trace file into memory
    vector<Trace> traces;
    Trace trace;
    while (fin.read(reinterpret_cast<char*>(&trace), sizeof(Trace))) {
        traces.push_back(trace);
    }
    
    cout << "Loaded " << traces.size() << " requests." << '\n';
    cout << "Starting benchmark..." << '\n';
    
    // Open CSV file for writing
    ofstream csv_out("results/benchmark_results.csv");
    if (!csv_out) {
        cerr << "Error: Could not open benchmark_results.csv for writing!" << '\n';
        return 1;
    }
    
    // Write CSV header
    csv_out << "Policy,CacheCapacity,TotalAccesses,Hits,Misses,HitRate\n";
    
    // Loop over cache capacities from 10,000 to 100,000 blocks in steps of 10,000
    for (int cap = 10000; cap <= 100000; cap += 10000) {
        cout << "Running benchmark step: Cache Capacity = " << cap << '\n';
        
        vector<CachePolicy*> policies = {
            new LRUCache(cap),
            new LFUCache(cap),
            new ARCCache(cap),
            new TwoQCache(cap)
        };
        
        for (CachePolicy* policy : policies) {
            cout << "  Simulating policy: " << policy->get_name() << "..." << '\n';
            run_cache_simulation_benchmark(policy, traces, cap, csv_out);
            delete policy; // Free memory
        }
        csv_out.flush(); // Flush data after each capacity step
    }
    
    csv_out.close();
    cout << "Benchmark completed successfully" << '\n' << "Results written to results/benchmark_results.csv" << '\n';
    
    return 0;
}
