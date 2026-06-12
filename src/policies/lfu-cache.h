#ifndef LFU_CACHE_H
#define LFU_CACHE_H

#include <unordered_map>
#include <list>
#include <tuple>
#include <iterator>
#include <cstdint> // For uint64_t compatibility with traces

using namespace std;

// O(1) LFU Cache implementation using a Hash Map and Frequency Lists
class LFUCache {
private:
    // We use uint64_t for key because disk block addresses (LBAs) are large.
    // maps key -> tuple of {value, frequency, iterator to key's position in freqList}
    unordered_map<uint64_t, tuple<int, int, list<uint64_t>::iterator>> storage;
    
    // maps frequency -> list of keys with that frequency (newest/MRU at the back)
    unordered_map<int, list<uint64_t>> freqList;

    int max_size;
    int min_freq;

public:
    // Constructor to initialize LFU cache capacity
    LFUCache(int capacity) {
        max_size = capacity;
        min_freq = 0;
    }
    
    // Look up key in O(1). Returns value, or -1 if not found.
    // Promotes the accessed key's frequency by 1.
    int get(uint64_t key) {
        if (max_size <= 0) return -1;
        
        auto it = storage.find(key);
        if (it == storage.end()) {
            return -1; // Cache Miss
        }

        // Cache Hit: Extract value, current frequency, and list iterator
        auto [value, freq, list_it] = it->second;

        // Remove the key from its current frequency list
        freqList[freq].erase(list_it);
        
        // If the frequency list is empty, clean it up
        if (freqList[freq].empty()) {
            freqList.erase(freq);
            // If the deleted frequency was the minimum, increment the minimum frequency
            if (freq == min_freq) {
                min_freq++;
            }
        }

        // Promote frequency
        freq++;
        freqList[freq].push_back(key); // Add to new frequency list
        
        // Update storage with new frequency and the new iterator (end of the list)
        storage[key] = {value, freq, prev(freqList[freq].end())};
        
        return value;
    }
    
    // Insert/update key and value in O(1).
    // Evicts least frequently (and oldest) used key if capacity is full.
    void put(uint64_t key, int value) {
        if (max_size <= 0) return;

        // If key already exists: update value and promote frequency
        if (storage.find(key) != storage.end()) {
            get(key); // Handles frequency promotion
            std::get<0>(storage[key]) = value; // Update value in the tuple
            return;
        }

        // If cache is full, evict LFU item
        if (storage.size() == max_size) {
            auto it = freqList.find(min_freq);
            uint64_t evict_key = it->second.front(); // LRU item in the min_freq list
            
            storage.erase(evict_key);
            it->second.pop_front();
            
            if (it->second.empty()) {
                freqList.erase(min_freq);
            }
        }

        // Insert new key with frequency 1
        freqList[1].push_back(key);
        storage[key] = {value, 1, prev(freqList[1].end())};
        min_freq = 1;
    }

    // Helper function to clear the cache
    int size() const noexcept {
        return storage.size();
    }

    int capacity() const noexcept {
        return max_size;
    }

    bool empty() const noexcept {
        return storage.empty();
    }
    
    void clear() {
        storage.clear();
        freqList.clear();
        min_freq = 0;
    }
};

#endif /* LFU_CACHE_H */