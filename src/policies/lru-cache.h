#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <utility>
#include <cstdint>
#include <string>
#include "cache-policy.h"

using namespace std;

// LRU Cache inheriting from CachePolicy
class LRUCache : public CachePolicy {
private:
    // Key is the 64-bit block address (LBA)
    // Maps key -> {cached value, iterator to the linked list node}
    unordered_map<uint64_t, pair<int, list<uint64_t>::iterator>> storage;
    
    int max_size = 0;
    
    // Doubly-linked list storing keys to track access recency
    // LRU is at the front, MRU is at the back
    list<uint64_t> age;

public:
    // Constructor
    LRUCache(int capacity) {
        max_size = capacity;
    }
    
    // Returns value if key is in cache (and moves key to back/MRU)
    // Returns -1 if it's a Cache Miss
    int get(uint64_t key) override {
        auto it = storage.find(key);
        if (it == storage.end()) {
            return -1; // Cache Miss
        }

        // Cache Hit: Move the node to the end of the list (most recently used)
        age.splice(age.end(), age, it->second.second);
        return it->second.first;
    }
    
    // Inserts or updates key-value pair in cache
    // Evicts the least recently used key if capacity is full
    void put(uint64_t key, int value) override {
        if (max_size <= 0) return;

        auto it = storage.find(key);
        if (it != storage.end()) {
            // Key already in cache: update value and move to back (MRU)
            it->second.first = value;
            age.splice(age.end(), age, it->second.second);
            return;
        }

        // Cache is full: evict LRU element from the front
        if (storage.size() == max_size) {
            uint64_t lru_key = age.front();
            storage.erase(lru_key);
            age.pop_front();
        }

        // Add new key to back of list and insert into map
        age.push_back(key);
        storage[key] = {value, prev(age.end())};
    }

    // Helper to return policy name
    string get_name() const override {
        return "LRU";
    }

    // Clear the cache
    void clear() override {
        storage.clear();
        age.clear();
    }

    // Helper functions for capacity check
    int size() const noexcept {
        return storage.size();
    }

    int capacity() const noexcept {
        return max_size;
    }

    bool empty() const noexcept {
        return storage.empty();
    }
};

#endif // LRU_CACHE_H