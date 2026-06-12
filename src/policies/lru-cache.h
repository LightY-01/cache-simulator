#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <utility>
#include <cstdint> // For uint64_t compatibility with traces

using namespace std;

// LRU Cache implementation for cache simulation
class LRUCache {
private:
    // Key is the 64-bit block address (LBA).
    // Maps key -> {cached value, iterator to the linked list node}.
    unordered_map<uint64_t, pair<int, list<uint64_t>::iterator>> storage;
    
    int max_size = 0;
    
    // Doubly-linked list storing keys to track access recency.
    // Least recently used is at the front; most recently used is at the back.
    list<uint64_t> age;

public:
    // Constructor
    LRUCache(int capacity) {
        max_size = capacity;
    }
    
    // Returns value if key is in cache (and moves key to back/MRU).
    // Returns -1 if it's a Cache Miss.
    int get(uint64_t key) {
        auto it = storage.find(key);
        if (it == storage.end()) {
            return -1; // Cache Miss
        }

        // Cache Hit: Move the node to the end of the list (most recently used)
        age.splice(age.end(), age, it->second.second);
        return it->second.first;
    }
    
    // Inserts or updates key-value pair in cache.
    // Evicts the least recently used key if capacity is full.
    void put(uint64_t key, int value) {
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

    // Helper functions for testing
    int size() const noexcept {
        return storage.size();
    }

    int capacity() const noexcept {
        return max_size;
    }

    bool empty() const noexcept {
        return storage.empty();
    }

    void clear() noexcept {
        storage.clear();
        age.clear();
    }
};

#endif