# Algorithm Battle – Which Data Structure Wins? (Product ID Search)

**Course:** Data Structures and Algorithms (21CS201J) – Assessment Task 3
**Language:** C++17 | **SDG:** 9 – Industry, Innovation and Infrastructure

## Build & Run

```bash
g++ -std=c++17 -Wall -o product_search main.cpp
./product_search            # Linux / macOS
product_search.exe          # Windows (cmd / PowerShell: .\product_search.exe)
```

The program (1) inserts 40 products into a BST, an AVL tree and a hash table, (2) runs 5 successful + 5 unsuccessful searches on all three, and (3) opens a live menu for inserting/searching new IDs during the viva.

**Counting rule:** BST/AVL = 1 comparison per node visited; Hash = 1 probe per slot examined.

---

## Part A – Problem Selection

**Problem:** Product ID Search in an e-commerce warehouse. Each record stores `productID`, `productName`, `category`, `price`.

**Why it matters (SDG 9):** Modern supply chains depend on digital inventory infrastructure. Every barcode scan, order pick and stock check is a lookup by Product ID. Slow lookups create latency at scale, wasting compute and worker time. Choosing the right data structure is a low-cost way to build more efficient, resilient logistics infrastructure.

**Why these three structures:**
- **BST** – simple ordered structure, but performance depends on insertion order.
- **AVL Tree** – a BST that stays balanced, guaranteeing logarithmic height.
- **Hash Table** – trades ordering for near-constant-time exact lookup.

**Dataset (40 records):** IDs `1001–1020` were added as a sequential bulk batch (worst case for a plain BST), followed by 20 scattered IDs (`1337 … 9921`).

---

## Part B – Implementation Overview

| Structure | How it works | Search | Insert | Space |
|---|---|---|---|---|
| **BST** | Left < node < right. Insert/search walk from the root, comparing at each node. No balancing. | Avg O(log n), **worst O(n)** | Avg O(log n), **worst O(n)** | O(n) |
| **AVL Tree** | BST + balance factor (`height(left) − height(right)`) at every node. If it leaves {−1, 0, +1}, rotations (LL, RR, LR, RL) restore balance. | **O(log n)** always | **O(log n)** (≤ 2 rotations per insert) | O(n) |
| **Hash Table (Linear Probing)** | `index = id % 53`. If occupied, try `index+1, +2, …` (wrapping). Search stops at the key or an empty slot. | Avg O(1), worst O(n) | Avg O(1), worst O(n) | O(m) table size (m = 53) |

Rotation summary: **LL → right rotate**, **RR → left rotate**, **LR → left rotate child, then right rotate**, **RL → right rotate child, then left rotate**.

---

## Part C & D – Experimental Results (40 records, deterministic)

### Search comparisons

| Test | Product ID | BST | AVL | Hash (probes) |
|---|---|---|---|---|
| Successful | 1001 | 1 | 5 | 1 |
| Successful | 1010 | 10 | 4 | 1 |
| Successful | 1020 | 20 | 4 | 1 |
| Successful | 6391 | 24 | 4 | 1 |
| Successful | 9921 | 24 | 5 | 12 |
| **Successful average** | | **15.80** | **4.40** | **3.20** |
| Unsuccessful | 1021 | 24 | 6 | 13 |
| Unsuccessful | 1500 | 24 | 6 | 11 |
| Unsuccessful | 3000 | 24 | 5 | 1 |
| Unsuccessful | 9999 | 24 | 5 | 1 |
| Unsuccessful | 500 | 1 | 5 | 4 |
| **Unsuccessful average** | | **19.40** | **5.40** | **6.00** |

### Summary

| Metric | BST | AVL Tree | Hash Table |
|---|---|---|---|
| Avg comparisons – successful | 15.80 | 4.40 | **3.20** |
| Avg comparisons – unsuccessful | 19.40 | 5.40 | 6.00 |
| Total insertion comparisons (40 inserts) | 647 (≈16.2/insert) | 165 (≈4.1/insert) | – |
| Tree height (nodes) | **26** | **6** | – |
| Rotations performed | – | 23 | – |
| Collisions during insertion | – | – | 172 (load factor 0.75, size 53) |
| Sorted output (in-order) | Yes | Yes | No |

### Overall observation
- **BST degenerated** into an almost linear chain (height 26 for 40 nodes) because IDs 1001–1020 arrived in ascending order. The theoretical minimum height for 40 nodes is 6.
- **AVL kept height at 6** using 23 rotations, which reduced successful search cost by about 3.6× and unsuccessful search cost by about 3.6× compared with the BST.
- **Hash was fastest for most successful lookups** (1 probe for 4 of 5 tests). However, the sequential batch (slots 47–52 and 0–13) created a long contiguous cluster, and scattered IDs that hashed into it needed many probes (e.g., 9921 took 12). This is **primary clustering**, and it also made some unsuccessful searches (1021, 1500) expensive.
- Hash has no ordering, so range queries are not supported.

---

## Part E – Final Verdict

**Winner for pure Product ID lookup: Hash Table.** On exact-ID search it had the best successful average (3.20 vs 4.40 AVL vs 15.80 BST) and its best-case behavior is constant time, which fits barcode-scan scenarios.

**But the AVL Tree is the safest all-round choice.** Its worst case is guaranteed O(log n) (5–6 comparisons here for every query, hit or miss), it gave better unsuccessful-search performance than the hash table in this dataset (5.40 vs 6.00), and it keeps data sorted.

| Situation | Best choice |
|---|---|
| Instant exact-ID barcode scans, no ordering needed, keys well spread | **Hash Table** (use a lower load factor or better hash/probing for sequential IDs) |
| Need sorted reports, range queries (e.g., IDs 1005–1015, price bands), predictable worst case | **AVL Tree** |
| Tiny dataset, random insertion order, simplest code | BST (never for sorted/bulk inserts) |

**Recommendation:** Use a hash table for the live scanning path and an AVL tree for reporting/range queries. Never use a plain BST when IDs arrive in sequential batches.

---

## Viva Prep Cheat Sheet

**1. What is primary clustering in linear probing?**
Colliding keys fill consecutive slots, forming a long run that grows whenever any key hashes into it. In our data, IDs 1001–1020 occupy slots 47–52 and 0–13 contiguously, so later keys such as 9921 needed 12 probes.

**2. Why is the table size prime (53)?**
A prime modulus shares no common factors with typical ID patterns or strides, so keys spread more uniformly across slots. A composite size (e.g., 50 or 60) would map IDs with common multiples into the same few slots.

**3. When does an AVL tree rotate, and which rotation is used?**
It rotates when a node's balance factor becomes +2 or −2 after an insertion. LL → right rotation, RR → left rotation, LR → left then right, RL → right then left.

**4. What is the worst case for a BST and why did it happen here?**
Inserting already-sorted keys makes every node a right child, so the tree becomes a linked list with O(n) height and search. Our sequential IDs 1001–1020 pushed the BST height to 26 for 40 nodes.

**5. Why does the hash table win exact search but lose for range reports?**
Hashing scatters keys pseudo-randomly, giving O(1) average lookup but no ordering, so "all IDs between 1005 and 1015" requires scanning the whole table. An AVL tree stores keys sorted, so a range query is O(log n + k).

---

## Files

| File | Purpose |
|---|---|
| `main.cpp` | Complete implementation (BST, AVL, Hash, experiment, viva menu) |
| `README.md` | This report |
