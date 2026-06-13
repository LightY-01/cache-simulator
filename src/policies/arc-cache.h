#ifndef ARC_CACHE_H
#define ARC_CACHE_H

#include <iostream>
#include <unordered_map>
#include <list>
#include <cstdint>
#include <string>
#include "cache-policy.h"

using namespace std;

// This tells the hash map exactly which list the key is currently sitting in
enum State { IN_T1, IN_T2, IN_B1, IN_B2 };

// The Custom Node
struct Node {
    int value;                  // The actual cached data (only relevant if IN_T1 or IN_T2)
    State state;                // Where is this key right now?
    list<uint64_t>::iterator ptr; // An O(1) pointer directly to the key in its specific list
};

// Adaptive Replacement Cache (ARC) inheriting from CachePolicy
class ARCCache : public CachePolicy {
private:
    // The Four Lists (Storing KEYS ONLY)
    list<uint64_t> T1;
    list<uint64_t> T2;
    list<uint64_t> B1;
    list<uint64_t> B2;

    // The Master Map
    // ONE map rules them all. Given a key, you instantly have its value, its state, and its list iterator
    unordered_map<uint64_t, Node> cache_map;

    int capacity;
    int p; // Adaptive parameter: target size for T1

    // Core helper method to balance the size of the lists and evict elements
    void replace(bool hit_in_b2) {
        // Safe check: If T1 is not empty and either (T2 is empty OR we meet the target adaptation condition)
        if (!T1.empty() && (T2.empty() || T1.size() > p || (hit_in_b2 && T1.size() == p))) {
            uint64_t lru_t1 = T1.front();
            T1.pop_front();
            B1.push_back(lru_t1);
            cache_map[lru_t1].state = IN_B1;
            cache_map[lru_t1].ptr = prev(B1.end());
        } else if (!T2.empty()) {
            uint64_t lru_t2 = T2.front();
            T2.pop_front();
            B2.push_back(lru_t2);
            cache_map[lru_t2].state = IN_B2;
            cache_map[lru_t2].ptr = prev(B2.end());
        }
    }

public:
    // Constructor
    ARCCache(int c) {
        capacity = c;
        p = 0;
    }
    
    // Look up key in the cache. Returns value or -1 if not found.
    int get(uint64_t key) override {
        auto it = cache_map.find(key);
        if (it == cache_map.end()) {
            return -1; // Cache Miss
        }
        
        Node& node = it->second;
        
        if (node.state == IN_T1) {
            // Move to MRU of T2
            T2.splice(T2.end(), T1, node.ptr);
            node.state = IN_T2;
            return node.value;
        } else if (node.state == IN_T2) {
            // Already in T2, just move to MRU
            T2.splice(T2.end(), T2, node.ptr);
            return node.value;
        }
        return -1;
    }
    
    // Insert/update key and value. Evicts elements if capacity is full.
    void put(uint64_t key, int value) override {
        if (capacity <= 0) return;

        auto it = cache_map.find(key);

        if (it != cache_map.end()) {
            Node &node = it->second;
            if (node.state == IN_T1) {
                T2.splice(T2.end(), T1, node.ptr);
                node.state = IN_T2;
                node.value = value;
            } else if (node.state == IN_T2) {
                T2.splice(T2.end(), T2, node.ptr);
                node.value = value;
            } else if (node.state == IN_B1) {
                // Adapt target size p upwards because of a hit in B1 (recency ghost list)
                int delta = (B1.size() >= B2.size()) ? 1 : B2.size() / B1.size(); 
                p = min(capacity, p + delta);
                replace(false);
                T2.splice(T2.end(), B1, node.ptr);
                node.state = IN_T2;
                node.value = value;
            } else if (node.state == IN_B2) {
                // Adapt target size p downwards because of a hit in B2 (frequency ghost list)
                int delta = (B2.size() >= B1.size()) ? 1 : B1.size() / B2.size(); 
                p = max(0, p - delta);
                replace(true);
                T2.splice(T2.end(), B2, node.ptr);
                node.state = IN_T2;
                node.value = value;
            }
            return;
        }

        // Handle cache miss (completely new key)
        if (T1.size() + B1.size() == capacity) {
            if (T1.size() < capacity) {
                uint64_t evict_key = B1.front();
                B1.pop_front();
                replace(false);
                cache_map.erase(evict_key);
            } else {
                uint64_t evict_key = T1.front();
                T1.pop_front();
                cache_map.erase(evict_key);
            }
        } else if (T1.size() + T2.size() + B1.size() + B2.size() == 2 * capacity) {
            uint64_t evict_key = B2.front();
            B2.pop_front();
            replace(false); // New key is NOT in B2, so replace(false) should be called
            cache_map.erase(evict_key);
        } else if (T1.size() + T2.size() == capacity) {
            replace(false);
        } 
        
        T1.push_back(key);
        cache_map[key] = {value, IN_T1, prev(T1.end())};
    }

    // Helper to return policy name
    string get_name() const override {
        return "ARC";
    }

    // Helper function to clear the cache and reset adaptation parameter
    void clear() override {
        T1.clear();
        T2.clear();
        B1.clear();
        B2.clear();
        cache_map.clear();
        p = 0;
    }
};

#endif // ARC_CACHE_H
