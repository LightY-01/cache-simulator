#ifndef TWOQ_CACHE_H
#define TWOQ_CACHE_H

#include <unordered_map>
#include <list>
#include <cstdint>
#include <string>
#include <algorithm>
#include "cache-policy.h"

using namespace std;

// Represents the state of a key in the 2Q cache
enum TwoQState {A1_IN, A1_OUT, AM};

// Custom Node for 2Q cache
struct TwoQNode { // Note: Node and State is already declared in arc, so renamed them to TwoQNode and TwoQState
    int value;
    TwoQState state;
    list<uint64_t>::iterator ptr; // O(1) pointer to the key in its corresponding list
};

// Two-Queue (2Q) Cache Policy inheriting from CachePolicy
class TwoQCache : public CachePolicy {
private:
    list<uint64_t> A1_in;  // FIFO queue for first-time accesses
    list<uint64_t> A1_out; // FIFO ghost queue for tracking evicted keys
    list<uint64_t> Am;     // LRU queue for frequently accessed pages

    unordered_map<uint64_t, TwoQNode> cache_map;

    int capacity;
    int A1in_cap;
    int A1out_cap;

    // Reclaims a memory page slot if we are at capacity
    // Implementation as per "Two-Queue Caching" by J. Moreira and S. Thanukos
    void reclaim() {
        // Total active pages in memory is |A1_in| + |Am|
        if (A1_in.size() + Am.size() < capacity) {
            return; // We still have free memory slots
        }

        if (A1_in.size() > A1in_cap) {
            // Evict the oldest key from A1_in to A1_out (ghost queue)
            uint64_t evict_key = A1_in.front();
            A1_in.pop_front();
            
            A1_out.push_back(evict_key);
            cache_map[evict_key].state = A1_OUT;
            cache_map[evict_key].ptr = prev(A1_out.end());

            // If A1_out exceeds its ghost limit, discard the oldest ghost key
            if (A1_out.size() > A1out_cap) {
                uint64_t out_evict = A1_out.front();
                A1_out.pop_front();
                cache_map.erase(out_evict);
            }
        } else {
            // Evict the LRU page out of Am and discard it completely
            uint64_t evict_key = Am.front();
            Am.pop_front();
            cache_map.erase(evict_key);
        }
    }

public:
    TwoQCache(int c) {
        capacity = c;
        A1in_cap = max(1, c / 4);     // Kin: usually 25% of capacity
        A1out_cap = max(1, c / 2);    // Kout: usually 50% of capacity
    }
    
    // Look up key, returns value, or -1 if not found
    int get(uint64_t key) override {
        if (capacity <= 0) return -1;

        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return -1; // Cache Miss
        }

        TwoQNode& node = it->second;
        if (node.state == A1_OUT) {
            return -1; // Ghost hit (data is not in memory)
        }
        
        if (node.state == AM) {
            // Move to MRU position of Am (most recently used)
            Am.splice(Am.end(), Am, node.ptr);
        }
        // If state is A1_IN, standard 2Q does nothing to its position

        return node.value;
    }

    // Insert or update key-value pair
    void put(uint64_t key, int value) override {
        if (capacity <= 0) return;

        auto it = cache_map.find(key);
        if (it != cache_map.end()) {
            TwoQNode& node = it->second;
            node.value = value;

            if (node.state == AM) {
                // Move to MRU of Am
                Am.splice(Am.end(), Am, node.ptr);
            }
            else if (node.state == A1_OUT) {
                // Hit on A1_out: promote to Am since it's accessed a second time
                A1_out.erase(node.ptr);
                
                // Reclaim a memory slot before reading it into memory
                reclaim();
                
                Am.push_back(key);
                node.state = AM;
                node.ptr = prev(Am.end());
            }
            // Note: If state is A1_IN, do nothing to its position - Standard 2Q behaviour.
            return;
        }

        // Cache miss: reclaim a page slot and add to A1_in
        reclaim();
        A1_in.push_back(key);
        cache_map[key] = {value, A1_IN, prev(A1_in.end())};
    }

    // Returns the policy name
    string get_name() const override {
        return "2Q";
    }

    // Clear all queues and map
    void clear() override {
        A1_in.clear();
        A1_out.clear();
        Am.clear();
        cache_map.clear();
    }
};

#endif // TWOQ_CACHE_H
