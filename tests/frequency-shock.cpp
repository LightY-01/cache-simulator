#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"
#include "../src/policies/2q-cache.h"

using namespace std;

// This test shows LFU's main weakness: Frequency Shock / Cache Pollution
// We access blocks 1-10 a lot to give them a high access frequency.
// Then we switch to accessing blocks 11-20.
// LFU gets 0% hit rate because it keeps holding the stale high-frequency blocks.
// Other policies adapt quickly to the new working set and get high hit rates.
int main() {
    cout << "=== Frequency Shock Test ===" << endl;
    
    int cache_size = 10;
    int burst_count = 100;
    int test_accesses = 100;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size),
        new TwoQCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        // Phase 1: Repeatedly access blocks 1-10 to build high frequency
        for (int f = 0; f < burst_count; f++) {
            for (uint64_t block = 1; block <= 10; block++) {
                policy->put(block, 1);
            }
        }
        
        // Phase 2: Switch to accessing blocks 11-20
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
