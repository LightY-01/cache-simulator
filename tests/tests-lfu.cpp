#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"

using namespace std;

// This test shows LFU's worst-case scenario: Frequency Shock (Cache Pollution).
// Phase 1: Access blocks 1-10 repeatedly (frequency = 100).
// Phase 2: Switch to accessing blocks 11-20.
// LFU gets 0% hit rate because it won't evict the stale high-frequency blocks.
// LRU and ARC adapt quickly to the new working set and get a high hit rate.
int main() {
    cout << "=== LFU Adversarial Test (Frequency Shock) ===" << endl;
    
    int cache_size = 10;
    int burst_count = 100;
    int test_accesses = 100;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        // Phase 1: Build frequency = 100 for blocks 1-10
        for (int f = 0; f < burst_count; f++) {
            for (uint64_t block = 1; block <= 10; block++) {
                policy->put(block, 1);
            }
        }
        
        // Phase 2: Shift working set completely to blocks 11-20
        int hits = 0;
        for (int i = 0; i < test_accesses; i++) {
            uint64_t block = 11 + (i % 10);
            if (policy->get(block) != -1) {
                hits++;
            } else {
                policy->put(block, 1);
            }
        }
        cout << policy->get_name() << " Phase 2 Hit Rate: " << ((double)hits / test_accesses) * 100.0 << "%" << endl;
        delete policy;
    }
    return 0;
}
