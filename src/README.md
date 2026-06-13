# Source Code Architecture

## 📌 Overview
This directory contains the core application logic and the data structures for the Cache Replacement Policy Simulator. The system is built using Object-Oriented Principles to ensure strict interface compliance and ease of testing.  

---

## 📂 Structure

* **`policies/cache-policy.h`:** The foundational base class. It defines the purely virtual `get` and `put` methods that all cache policies must implement, allowing the main orchestrator to test different algorithms polymorphically.  
* **`lru-cache.h`:** Production-ready implementation of the Least Recently Used policy using an `unordered_map` and `std::list`.  
* **`lfu-cache.h`:** Highly optimized O(1) Least Frequently Used implementation utilizing frequency buckets and nested list iterators.  
* **`arc-cache.h`:** The Adaptive Replacement Cache utilizing custom `Node` structures and state enums (`IN_T1`, `IN_T2`, `IN_B1`, `IN_B2`) mapped to a single overarching hash map for amortized O(1) tracking.
