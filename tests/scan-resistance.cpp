#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"
#include "../src/policies/2q-cache.h"

using namespace std;

// This test shows 2Q's strength: Scan Resistance
// Workload steps:
// 1. Load keys 1-5
// 2. Load dummy keys 10-16 to push keys 1-5 to ghost list (A1_out / B1)
// 3. Re-access keys 1-5 (ghost hits promote them to high-frequency state Am / T2)
// 4. Scan 100 unique keys (100-199) to try and flush the cache
// 5. Request keys 1-5 again to see if they survived the scan
// LRU, LFU, and ARC get 0% hit rate (flushed by the scan)
// 2Q gets 100% hit rate (Am is protected from scans by the A1in cap)
int main() {
    cout << "=== Scan Resistance Test ===" << endl;
    
    int cache_size = 10;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size),
        new TwoQCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        // 1. Load keys 1-5
        for (int i = 1; i <= 5; i++) {
            policy->put(i, i * 10);
        }
        
        // 2. Load dummy keys to evict keys 1-5 to ghost lists
        for (int i = 10; i <= 16; i++) {
            policy->put(i, i * 10);
        }
        
        // 3. Re-access keys 1-5 to trigger ghost hits
        for (int i = 1; i <= 5; i++) {
            if (policy->get(i) == -1) {
                policy->put(i, i * 10);
            }
        }
        
        // 4. Run large scan of unique keys
        for (int i = 100; i < 200; i++) {
            policy->put(i, i * 10);
        }
        
        // 5. Check if keys 1-5 survived
        int hits = 0;
        int accesses = 0;
        for (int i = 1; i <= 5; i++) {
            accesses++;
            if (policy->get(i) != -1) {
                hits++;
            }
        }
        
        cout << policy->get_name() << " Hit Rate after Scan: " << ((double)hits / accesses) * 100.0 << "%" << endl;
        delete policy;
    }
    return 0;
}
