#ifndef LFU_CACHE_LOGN_H
#define LFU_CACHE_LOGN_H

#include <unordered_map>
#include <set>
#include <tuple>
#include <utility>
#include <cstdint> // For uint64_t compatibility with traces

using namespace std;

// O(log N) LFU Cache implementation using std::set
class LFUCache {
private:
    // Key is the 64-bit block address (LBA).
    // Maps key -> cached value.
    unordered_map<uint64_t, int> storage;
    
    // Maps key -> {access_frequency, last_accessed_time}
    unordered_map<uint64_t, pair<int, int>> key_freq;
    
    // Set to keep track of frequency and recency.
    // Set stores tuples of <frequency, last_accessed_time, key>
    // It keeps them sorted, so the least frequent (and oldest) is at the beginning.
    set<tuple<int, int, uint64_t>> freq_set;
    
    int max_size = 0;
    int current_time = 0; // Simple logical timestamp incremented on each access

public:
    // Constructor
    LFUCache(int capacity) {
        max_size = capacity;
        current_time = 0;
    }
    
    // Retrieves value from cache and updates its frequency and access time.
    // Returns -1 if key is not found (Cache Miss).
    int get(uint64_t key) {
        if (max_size <= 0) return -1;
        
        current_time++;
        auto it = storage.find(key);
        if (it == storage.end()) {
            return -1; // Cache Miss
        }

        // Cache Hit: Get current frequency and access time
        int freq = key_freq[key].first;
        int old_time = key_freq[key].second;
        
        // Remove the old tuple from the sorted set
        freq_set.erase({freq, old_time, key});
        
        // Update frequency and timestamp
        key_freq[key].first = freq + 1;
        key_freq[key].second = current_time;
        
        // Re-insert updated tuple into the set
        freq_set.insert({freq + 1, current_time, key});
        
        return it->second;
    }
    
    // Inserts or updates a key-value pair.
    // Evicts the least frequently used (and oldest) key if cache capacity is full.
    void put(uint64_t key, int value) {
        if (max_size <= 0) return;
        
        current_time++;
        auto it = storage.find(key);
        if (it != storage.end()) {
            // Key already exists: update its value
            it->second = value;
            
            // Get old frequency and timestamp
            int freq = key_freq[key].first;
            int old_time = key_freq[key].second;
            
            // Remove old tuple and insert updated one
            freq_set.erase({freq, old_time, key});
            key_freq[key].first = freq + 1;
            key_freq[key].second = current_time;
            freq_set.insert({freq + 1, current_time, key});
            return;
        }

        // If cache capacity is reached, evict LFU item
        if (storage.size() == max_size) {
            // The first element in the set is the LFU (and LRU if frequencies match)
            auto [least_freq, old_time, rm_key] = *freq_set.begin();
            
            storage.erase(rm_key);
            key_freq.erase(rm_key); // Clean up frequency map to prevent memory leak
            freq_set.erase({least_freq, old_time, rm_key});
        }
        
        // Insert new item
        storage[key] = value;
        key_freq[key] = {1, current_time};
        freq_set.insert({1, current_time, key});
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
        key_freq.clear();
        freq_set.clear();
    }
};

#endif