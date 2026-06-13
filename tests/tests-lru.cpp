#include <iostream>
#include <vector>
#include "../src/policies/cache-policy.h"
#include "../src/policies/lru-cache.h"
#include "../src/policies/lfu-cache.h"
#include "../src/policies/arc-cache.h"

using namespace std;

// This test shows LRU's worst-case scenario: Sequential Thrashing
// Access loop of size N = cache_size + 1
// All standard policies will get 0% hit rate
int main() {
    cout << "=== LRU Adversarial Test (Sequential Thrashing) ===" << endl;
    
    int cache_size = 10;
    int N = cache_size + 1; // 11 unique blocks (0 to 10)
    int total_accesses = 100;
    
    vector<CachePolicy*> policies = {
        new LRUCache(cache_size),
        new LFUCache(cache_size),
        new ARCCache(cache_size)
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
