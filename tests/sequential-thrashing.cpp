#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"
#include "../src/policies/2q-cache.h"

using namespace std;

// This test shows Sequential Thrashing
// We access blocks in a loop (0, 1, 2, ..., N-1, 0, 1, 2, ...)
// N is cache_size + 1, so the loop is just slightly larger than the cache.
// LRU, LFU, and ARC get 0% hit rate because they evict the block we need next.
// 2Q gets a higher hit rate (56%) because its Am queue protects looping pages.
int main() {
    cout << "=== Sequential Thrashing Test ===" << endl;
    
    int cache_size = 10;
    int N = cache_size + 1; // Loop size is 11 (blocks 0 to 10)
    int total_accesses = 100;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size),
        new TwoQCache(cache_size)
    };
    
    for (CachePolicy* policy : policies) {
        int hits = 0;
        for (int i = 0; i < total_accesses; i++) {
            uint64_t block = i % N;
            if (policy->get(block) != -1) {
                hits++;
            } else {
                policy->put(block, 1);
            }
        }
        cout << policy->get_name() << " Hit Rate: " << ((double)hits / total_accesses) * 100.0 << "%" << endl;
        delete policy;
    }
    return 0;
}
