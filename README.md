# Cache Replacement Policy Simulator

## 📌 Overview
This project is a custom C++ simulator built from scratch to evaluate and benchmark advanced cache replacement policies. It goes beyond standard implementations by analyzing performance against real-world disk access traces and identifying adversarial workloads where specific algorithms fail or excel.  

## 🧠 Algorithmic Depth
Every implemented policy requires a distinct, complex composition of data structures. This project specifically highlights edge-case analysis, such as demonstrating LRU thrashing during sequential scans and proving how adaptive policies like ARC can seamlessly adjust to outperform it.  

## ⚙️ Core Replacement Policies
The simulator implements the following algorithms with a strict focus on optimal time and space complexity:

* **LRU (Least Recently Used):** Implemented using a Doubly Linked List (DLL) and an `unordered_map`. Operations involve moving accessed nodes to the front and evicting from the tail, ensuring O(1) time complexity for all operations.  
* **LFU (Least Frequently Used) O(1):** Utilizes a DLL of frequency buckets alongside `key->freq` and `key->node` maps. Maintains an O(1) runtime by tracking a `min_freq` variable and shifting keys between frequency buckets upon access.  
* **ARC (Adaptive Replacement Cache):** Maintains two primary queues: T1 for recent items and T2 for frequent items. Uses two ghost lists (B1 and B2) to track evicted items and an adaptive parameter *p* to dynamically shift capacity between recency and frequency based on workload behavior. Achieves an amortized O(1) time complexity.  
* **2Q (Two Queue):** Directs new items into a FIFO A1in queue. If an item is hit while residing in the A1out (FIFO ghost) queue, it is promoted to the Am queue, which is managed via LRU. Operates in O(1) time complexity.  

## 📊 Benchmarking & Dataset
* **Dataset Source:** Benchmarks are run against public UMass IoTrace disk access traces (traces.cs.umass.edu).  
* **Evaluation Metrics:** The system evaluates hit-rate versus cache-size, sweeping from 10% to 50% of the working set for each trace.  
* **Visualization:** Generates unified hit-rate curves comparing all four policies simultaneously.  
* **Adversarial Testing:** Constructs sequential scan workloads to purposefully trigger LRU thrashing, visualizing how adaptive algorithms react under stress.  
* **Complexity Analysis:** Includes a comprehensive table breaking down the time and space complexity per operation for all four algorithms.
