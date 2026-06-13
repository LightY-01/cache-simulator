#ifndef CACHE_POLICY_H
#define CACHE_POLICY_H

#include <cstdint>
#include <string>

using namespace std;

// Base class for all cache policies (OOP inheritance)
class CachePolicy {
public:
    // Look up key. Returns cached value, or -1 if not found.
    virtual int get(uint64_t key) = 0;
    
    // Insert/update key-value pair. Evicts elements if capacity is full.
    virtual void put(uint64_t key, int value) = 0;
    
    // Clear all elements and reset cache state
    virtual void clear() = 0;
    
    // Returns the name of the policy (e.g. "LRU", "LFU", "ARC")
    virtual string get_name() const = 0;
    
    // Virtual destructor is required for safe deletion of derived objects
    virtual ~CachePolicy() {}
};

#endif // CACHE_POLICY_H
