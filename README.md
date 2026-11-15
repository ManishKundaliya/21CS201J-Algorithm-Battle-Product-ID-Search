# Experimental Comparison of BST, AVL Tree, and Linear-Probing Hashing for Warehouse Product ID Lookup

- **Course:** 21CS201J – Data Structures and Algorithms (Term Work – Assessment Task 3)
- **Author:** Manish Jain (B.Tech CSE-AIML, Semester III, 2026–2027)
- **Institution:** SRM Institute of Science and Technology
- **Language:** C++17 (`g++ -std=c++17 -Wall`)
- **SDG Alignment:** SDG 9 – Industry, Innovation and Infrastructure (Design Alignment)

---

## Compilation and Execution

```bash
g++ -std=c++17 -Wall -o product_search main.cpp
./product_search.exe        # Windows (MINGW64 / Git Bash / PowerShell)
./product_search            # Linux / macOS
```

Executing the binary runs three stages sequentially:
1. Populates a **Binary Search Tree (BST)**, an **AVL Tree**, and an **Open-Addressed Hash Table** with the same 40-item retail inventory catalog while logging insertion metrics.
2. Executes **5 successful** and **5 unsuccessful** lookup queries across all three structures, counting every node comparison (trees) or slot probe (hash table).
3. Opens an interactive terminal menu for inserting new SKUs, rejecting duplicate IDs, and inspecting live lookup counts during evaluation.

---

## Part A – Application Context and Architecture Rationale

### 1. Operational Scenario
In an automated fulfillment center or retail checkout counter, barcode scanners read a numeric `productID` and query the backend catalog to fetch its `productName`, `category`, and `price`. Because incoming supplier shipments frequently arrive with consecutive SKU numbers (`1001` through `1020`) alongside irregular category codes (`1337` up to `9921`), the underlying storage structure must handle mixed arrival patterns without slowing down point-of-sale queries.

### 2. Relevance to SDG 9
Digital supply-chain systems form a core layer of modern industrial infrastructure (SDG 9). High-latency inventory lookups bottleneck warehouse picking and checkout throughput while forcing servers to burn extra CPU cycles per transaction. Evaluating algorithmic trade-offs at the data-structure level ensures responsive, resource-efficient inventory software without requiring heavier hardware.

### 3. Why These Three Structures Were Tested Together
- **Unbalanced BST:** Maintains sorted key order with minimal implementation overhead, making it a useful baseline for observing how consecutive supplier batches distort tree depth.
- **AVL Tree:** Enforces a strict balance condition (`|leftHeight - rightHeight| <= 1`) through local rotations, preserving logarithmic traversal regardless of input ordering.
- **Hash Table (Linear Probing):** Maps each `productID` directly to an array bucket via modulo arithmetic, sacrificing sorted traversal in exchange for near-constant-time exact matching.

---

## Part B – Implementation Design (`main.cpp`)

All three containers operate on identical `Product` structs (`id`, `name`, `category`, `price`) without external third-party libraries:

- **Binary Search Tree (`BST`):** Standard pointer-based nodes ordered by `id`. `insert()` and `search()` increment a counter on every visited node until reaching a match or a null child.
- **AVL Tree (`AVLTree`):** Extends each node with a cached `height` integer. After recursive insertion, balance factors outside `[-1, 1]` trigger single rotations (`rotateRight` for LL, `rotateLeft` for RR) or double rotations (`LR` and `RL`). Both trees support sorted catalog printing via in-order traversal.
- **Hash Table (`HashTable`):** Uses a fixed prime capacity of `53` slots (`load factor = 40 / 53 ≈ 0.75`) and hashes keys via `productID % 53`. Occupied slots advance linearly by `(index + step) % 53`, counting every inspected bucket as a probe.

---

## Part C – Recorded Experimental Outputs

### 1. Bulk Insertion Metrics (40 Unique SKUs)
- **BST Final Height:** `26` levels (`647` total comparisons across 40 insertions)
- **AVL Final Height:** `6` levels (`165` total comparisons; `23` rotations triggered)
- **Hash Table Collisions:** `172` cumulative collisions at `0.75` load factor (`size = 53`)

### 2. Lookup Benchmark Results

#### Existing Product IDs (Successful Queries)
| Queried ID | Catalog Match | BST Comparisons | AVL Comparisons | Hash Probes |
| :---: | :--- | :---: | :---: | :---: |
| `1001` | Found | 1 | 5 | 1 |
| `1010` | Found | 10 | 4 | 1 |
| `1020` | Found | 20 | 4 | 1 |
| `6391` | Found | 24 | 4 | 1 |
| `9921` | Found | 24 | 5 | 12 |
| **Mean** | — | **15.80** | **4.40** | **3.20** |

#### Missing Product IDs (Unsuccessful Queries)
| Queried ID | Catalog Match | BST Comparisons | AVL Comparisons | Hash Probes |
| :---: | :--- | :---: | :---: | :---: |
| `1021` | Not Found | 24 | 6 | 13 |
| `1500` | Not Found | 24 | 6 | 11 |
| `3000` | Not Found | 24 | 5 | 1 |
| `9999` | Not Found | 24 | 5 | 1 |
| `500` | Not Found | 1 | 5 | 4 |
| **Mean** | — | **19.40** | **5.40** | **6.00** |

### 3. Live Menu Verification Log
- **Duplicate Guard Check (`ID 1011`):** Attempting to re-insert `1011` was blocked cleanly across all three structures (`ID 1011 already exists - duplicate rejected`).
- **Live Query Check (`ID 1011` – `Screen Guard`, `Mobile`, `Rs.199.00`):** Required `11` comparisons in BST, `5` comparisons in AVL, and `1` direct probe in the Hash Table (bucket `4`).

### Terminal Execution Screenshots
![Benchmark Results](screenshots/benchmark_results.png)
![Live Viva Menu Demo](screenshots/viva_demo.png)

---

## Part D – Comparative Analysis

| Parameter | BST | AVL | Hashing |
| :--- | :--- | :--- | :--- |
| **Search comparisons** | Hit avg: `15.80` (1–24)<br>Miss avg: `19.40` (1–24) | Hit avg: `4.40` (4–5)<br>Miss avg: `5.40` (5–6) | Hit avg: `3.20` (1–12)<br>Miss avg: `6.00` (1–13) |
| **Insertion observations** | Required `647` comparisons; consecutive SKUs `1001–1020` chained down the right spine. | Required `165` comparisons; `23` rotations held the tree at optimal depth. | Logged `172` collisions at `0.75` occupancy; sequential SKUs packed into adjacent slots. |
| **Height** | `26` nodes | `6` nodes | – |
| **Collisions** | – | – | `172` |
| **Overall observation** | Unsuitable for batch-ordered SKUs due to right-branch skewing; supports sorted listing. | Tightest worst-case guarantee (`<= 6` steps for every lookup) plus native sorted traversal. | Fastest for direct SKU hits (`1` probe on 4 of 5 hits), though slot clustering penalizes misses; unordered. |

### Key Patterns Observed in the Data
1. **Right-Spine Skewing in BST:** Inserting `1001` through `1020` in ascending sequence forced each new SKU onto the right child of its predecessor, stretching height to `26` (far above the optimal `6` levels for 40 items). Consequently, deeper keys such as `6391` and `9921` each needed `24` node visits.
2. **Rotation Payoff in AVL:** Performing `23` rotations during insertion kept every search bounded between `4` and `6` comparisons—cutting average successful lookup cost by **72%** (`4.40` vs. `15.80`) relative to the unbalanced BST.
3. **Contiguous Slot Clustering in Linear Probing:** Because `1001 % 53 = 47` through `1020 % 53 = 13` filled buckets `47–52` and wrapped across `0–13` without gaps, later keys landing inside that occupied block suffered multi-step linear scans (`9921` required `12` probes; missing key `1021` walked `13` slots before hitting an empty bucket).

---

## Part E – Final Recommendation

**Selected Winner for Product ID Search: Hash Table (Linear Probing)**

For a barcode-driven Product ID lookup service where almost all queries check existing SKUs by exact ID, **Hashing delivers the best real-world throughput**:
- It achieved the lowest mean comparison count on successful lookups (**`3.20`** vs. **`4.40`** for AVL and **`15.80`** for BST), resolving **80% of valid test queries (`1001`, `1010`, `1020`, `6391`) in a single probe**.
- Reducing table occupancy below `0.55` (for instance, sizing the array to `79` or `83` slots for 40 products) or switching to double hashing would eliminate the primary clustering seen on `1021` and `1500`.

**When AVL Tree Should Be Chosen Instead:**  
If the inventory module also needs **sorted catalog exports** (such as printing items in ascending SKU order) or **range queries** (for example, listing all products priced or numbered between `1001` and `1010`), a Hash Table cannot answer without scanning and sorting the entire array. In those reporting workloads, the **AVL Tree** is the stronger engineering choice due to its guaranteed `<= 6` comparison ceiling and native in-order traversal.

---

## Repository Contents

```text
├── main.cpp            # C++17 source implementing BST, AVL Tree, Hash Table, benchmark, and viva menu
├── README.md           # Experimental report and comparative analysis
└── screenshots/        # Captured terminal outputs from local execution
```