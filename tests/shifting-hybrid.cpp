#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"
#include "../src/policies/2q-cache.h"

using namespace std;

// This test demonstrates ARC's adaptive behavior on a shifting hybrid workload
// 1. Warm up with repeating keys 1-5
// 2. Scan unique keys (100-149) which flushes LRU
// 3. Request keys 1-5 again
// 4. Shift working set to new repeating keys 6-10
// 5. Scan unique keys (200-249)
// 6. Request keys 6-10 again
int main() {
    cout << "=== Shifting Hybrid Workload Test ===" << endl;
    
    int cache_size = 10;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size),
        new TwoQCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        int hits = 0;
        int accesses = 0;
        
        // 1. Warm up with repeating keys 1-5
        for (int i = 0; i < 50; i++) {
            policy->put((i % 5) + 1, 1);
        }
        
        // 2. Scan unique keys 100-149
        for (int i = 100; i < 150; i++) {
            policy->put(i, 1);
        }
        
        // 3. Request keys 1-5 again
        for (int i = 0; i < 50; i++) {
            accesses++;
            if (policy->get((i % 5) + 1) != -1) {
                hits++;
            }
        }
        
        // 4. Shift to new repeating keys 6-10
        for (int i = 0; i < 50; i++) {
            policy->put((i % 5) + 6, 1);
        }
        
        // 5. Scan unique keys 200-249
        for (int i = 200; i < 250; i++) {
            policy->put(i, 1);
        }
        
        // 6. Request keys 6-10 again
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
