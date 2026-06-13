#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"

using namespace std;

// This test demonstrates ARC's adaptive superiority over LRU and LFU
// Workload:
// 1. Warm up with frequent keys 1-5
// 2. Scan of unique keys (100 to 149) which normally flushes LRU
// 3. Request keys 1-5 (LRU misses all, LFU and ARC hit)
// 4. Shift working set to new frequent keys 6-10
// 5. Scan of unique keys (200 to 249)
// 6. Request keys 6-10 (LFU misses all due to stale keys 1-5, LRU and ARC hit)
int main() {
    cout << "=== ARC Adaptivity Test (Shifting Hybrid Workload) ===" << endl;
    
    int cache_size = 10;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        int hits = 0;
        int accesses = 0;
        
        // 1. Warm up with frequent keys 1-5
        for (int i = 0; i < 50; i++) {
            policy->put((i % 5) + 1, 1);
        }
        
        // 2. Scan of unique keys (100 to 149)
        for (int i = 100; i < 150; i++) {
            policy->put(i, 1);
        }
        
        // 3. Test hits on keys 1-5 (LRU will fail, LFU and ARC will hit)
        for (int i = 0; i < 50; i++) {
            accesses++;
            if (policy->get((i % 5) + 1) != -1) {
                hits++;
            }
        }
        
        // 4. Shift working set to keys 6-10
        for (int i = 0; i < 50; i++) {
            policy->put((i % 5) + 6, 1);
        }
        
        // 5. Scan of unique keys (200 to 249)
        for (int i = 200; i < 250; i++) {
            policy->put(i, 1);
        }
        
        // 6. Test hits on new frequent keys 6-10 (LFU will fail due to cache pollution, LRU and ARC will hit)
        for (int i = 0; i < 50; i++) {
            accesses++;
            if (policy->get((i % 5) + 6) != -1) {
                hits++;
            }
        }
        
        cout << policy->get_name() << " Overall Hit Rate: " << ((double)hits / accesses) * 100.0 << "%" << endl;
        delete policy;
    }
    return 0;
}
