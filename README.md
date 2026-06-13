# Cache Replacement Policy Simulator

## 📌 Overview
This project is a custom C++ simulator built from scratch to evaluate and benchmark advanced cache replacement policies. It goes beyond standard implementations by analyzing performance against real-world disk access traces and identifying adversarial workloads where specific algorithms fail or excel.

The architecture utilizes Object-Oriented Programming (OOP) inheritance. All algorithms inherit from a base `CachePolicy` class, allowing a single, polymorphic `run_cache_simulation` function to dynamically test any policy against massive binary traces.

## 🧠 Algorithmic Depth
Every implemented policy requires a distinct, complex composition of data structures. This project specifically highlights edge-case analysis, such as demonstrating LRU thrashing during sequential scans and proving how adaptive policies like ARC can seamlessly adjust to outperform it.

## ⚙️ Core Replacement Policies
The simulator implements the following algorithms with a strict focus on optimal time and space complexity:

* **LRU (Least Recently Used):** Implemented using a Doubly Linked List (DLL) and an `unordered_map`. Operations involve moving accessed nodes to the front and evicting from the tail, ensuring O(1) time complexity for all operations.  
* **LFU (Least Frequently Used) O(1):** Utilizes a DLL of frequency buckets alongside `key->freq` and `key->node` maps. Maintains an O(1) runtime by tracking a `min_freq` variable and shifting keys between frequency buckets upon access.  
* **ARC (Adaptive Replacement Cache):** Maintains two primary queues: T1 for recent items and T2 for frequent items. Uses two ghost lists (B1 and B2) to track evicted items and an adaptive parameter *p* to dynamically shift capacity between recency and frequency based on workload behavior. Achieves an amortized O(1) time complexity.  
* **2Q (Planned):** Directs new items into a FIFO A1in queue. If an item is hit while residing in the A1out (FIFO ghost) queue, it is promoted to the Am queue, which is managed via LRU. Operates in O(1) time complexity.  

---

## 📊 Benchmarking & Dataset
* **Dataset Source:** Benchmarks are run against public UMass IoTrace disk access traces available in the [UMass Storage Repository](https://traces.cs.umass.edu/docs/traces/storage/). The raw `.spc.bz2` files, such as [Financial1.spc.bz2](https://skulddata.cs.umass.edu/traces/storage/Financial1.spc.bz2), are converted into packed binary format using a custom `parse-traces.py` script for high-performance reading in C++.  

### Simulation Results (`Financial1.bin` with 50,000 blocks)

| Policy | Hit Rate |
| :--- | :--- |
| **LRU** | 52.55% |
| **LFU** | 20.10% |
| **ARC** | 53.75% |

### Adversarial Testing
Constructs specialized workloads to purposefully trigger algorithm weaknesses:  
* **LRU Sequential Thrashing:** Results in a **0%** hit rate.  
* **LFU Frequency Shock:** Results in a **0%** hit rate for new phases.  
* **ARC Adaptivity:** Successfully achieves a **100%** overall hit rate during shifting hybrid workloads.  

---

## 🚀 How to Run

### 1. Parse the Trace Dataset
First, convert the downloaded UMass trace into a readable binary format using the Python utility:
```bash
python3 parse-traces.py
```
### 2. Compile and Run the Main Simulator
The main simulator acts as the orchestrator, running the UMass disk traces through all implemented policies using O3 optimization for maximum performance.

```bash
g++ -O3 main.cpp -o main
./main Financial1.bin
```
### 3. Compile and Run the Adversarial Tests
The edge-case analysis and logic verification tests are isolated in the tests/ directory.

```Bash
g++ -O3 tests/tests-lru.cpp -o test-lru && ./test-lru
g++ -O3 tests/tests-lfu.cpp -o test-lfu && ./test-lfu
g++ -O3 tests/tests-arc.cpp -o test-arc && ./test-arc
```
