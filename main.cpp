// =====================================================================
//  DSA (21CS201J) - Assessment Task 3: Algorithm Battle
//  "Which Data Structure Wins?"  ->  Product ID Search
//  Compares: Binary Search Tree (BST), AVL Tree, Hash Table (Linear Probing)
//  SDG 9: Industry, Innovation and Infrastructure
//
//  Compile : g++ -std=c++17 -Wall -o product_search main.cpp
//  Run     : product_search.exe   (Windows)   |   ./product_search (Linux/Mac)
//
//  COMPARISON-COUNTING RULE (use this in your viva):
//    BST / AVL : 1 comparison for every node we visit on the path.
//    Hash      : 1 probe for every table slot we examine.
// =====================================================================
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
using namespace std;

// ---------------------------------------------------------------------
// 1. RECORD STORED IN EVERY DATA STRUCTURE
// ---------------------------------------------------------------------
struct Product {
    int    productID;
    string productName;
    string category;
    double price;
};

void printProduct(const Product& p) {
    cout << "ID: " << p.productID << " | " << p.productName
         << " | " << p.category << " | Rs." << fixed << setprecision(2) << p.price << "\n";
}

// =====================================================================
// 2. BINARY SEARCH TREE (no balancing)
// =====================================================================
struct BSTNode {
    Product  data;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const Product& p) : data(p), left(nullptr), right(nullptr) {}
};

class BST {
public:
    BSTNode* root = nullptr;

    // Iterative insert. 'comparisons' = nodes visited on the way down.
    // Returns false if the ID already exists (duplicates are rejected).
    bool insert(const Product& p, int& comparisons) {
        comparisons = 0;
        BSTNode* newNode = new BSTNode(p);
        if (root == nullptr) { root = newNode; return true; }

        BSTNode* current = root;
        while (true) {
            comparisons++;                                   // compare with current node
            if (p.productID == current->data.productID) { delete newNode; return false; }
            if (p.productID < current->data.productID) {     // go LEFT
                if (current->left == nullptr) { current->left = newNode; return true; }
                current = current->left;
            } else {                                         // go RIGHT
                if (current->right == nullptr) { current->right = newNode; return true; }
                current = current->right;
            }
        }
    }

    // Search: returns pointer to Product (or nullptr) and the comparison count.
    const Product* search(int id, int& comparisons) const {
        comparisons = 0;
        BSTNode* current = root;
        while (current != nullptr) {
            comparisons++;
            if (id == current->data.productID) return &current->data;
            current = (id < current->data.productID) ? current->left : current->right;
        }
        return nullptr;
    }

    // Height counted in NODES on the longest root-to-leaf path (empty tree = 0).
    int height(BSTNode* node) const {
        if (node == nullptr) return 0;
        return 1 + max(height(node->left), height(node->right));
    }
    int height() const { return height(root); }

    // In-order traversal (Left, Root, Right) -> sorted order. Stops after 'limit' nodes.
    void inorder(BSTNode* node, int& printed, int limit) const {
        if (node == nullptr || printed >= limit) return;
        inorder(node->left, printed, limit);
        if (printed < limit) { cout << "   "; printProduct(node->data); printed++; }
        inorder(node->right, printed, limit);
    }
    void printFirst(int limit) const { int printed = 0; inorder(root, printed, limit); }
};

// =====================================================================
// 3. AVL TREE (self-balancing BST)
//    Balance Factor (BF) = height(left subtree) - height(right subtree)
//    Allowed BF values: -1, 0, +1.  If |BF| > 1 we rotate.
// =====================================================================
struct AVLNode {
    Product  data;
    AVLNode* left;
    AVLNode* right;
    int      h;          // height of this node (leaf = 1)
    AVLNode(const Product& p) : data(p), left(nullptr), right(nullptr), h(1) {}
};

class AVL {
public:
    AVLNode* root = nullptr;
    int rotationCount = 0;   // total rotations performed (useful for viva)

    int height(AVLNode* n) const { return n ? n->h : 0; }
    int height() const { return height(root); }

    int balanceFactor(AVLNode* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }
    void updateHeight(AVLNode* n) { n->h = 1 + max(height(n->left), height(n->right)); }

    // RR case -> single LEFT rotation.   (BF < -1 and new key went into RIGHT-RIGHT)
    //   x                    y
    //    (right child y)      (x left, z right)
    //     x -> y -> z   ==>      y has children x and z
    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;   // y becomes the new root of this subtree
        AVLNode* T = y->left;    // y's left child must move
        y->left  = x;            // x comes DOWN to the left of y
        x->right = T;            // T becomes x's right child
        updateHeight(x);         // x is lower now -> update it FIRST
        updateHeight(y);
        rotationCount++;
        return y;
    }

    // LL case -> single RIGHT rotation.  (BF > +1 and new key went into LEFT-LEFT)
    //   x -> y -> z (all on the left)   ==>   y has children z and x
    AVLNode* rotateRight(AVLNode* x) {
        AVLNode* y = x->left;
        AVLNode* T = y->right;
        y->right = x;
        x->left  = T;
        updateHeight(x);
        updateHeight(y);
        rotationCount++;
        return y;
    }

    // Recursive insert. 'comparisons' counts nodes visited on the way down.
    AVLNode* insert(AVLNode* node, const Product& p, int& comparisons, bool& inserted) {
        // Step 1: normal BST insert
        if (node == nullptr) { inserted = true; return new AVLNode(p); }

        comparisons++;
        if (p.productID == node->data.productID) { inserted = false; return node; }  // duplicate
        if (p.productID < node->data.productID)
            node->left  = insert(node->left,  p, comparisons, inserted);
        else
            node->right = insert(node->right, p, comparisons, inserted);

        // Step 2: update height of this ancestor on the way back UP
        updateHeight(node);

        // Step 3: compute balance factor and fix if |BF| > 1
        int bf = balanceFactor(node);

        // LL: left-heavy (BF > 1) and new key is in LEFT child's LEFT -> rotate RIGHT
        if (bf > 1 && p.productID < node->left->data.productID)
            return rotateRight(node);

        // RR: right-heavy (BF < -1) and new key is in RIGHT child's RIGHT -> rotate LEFT
        if (bf < -1 && p.productID > node->right->data.productID)
            return rotateLeft(node);

        // LR: left-heavy but new key is in LEFT child's RIGHT
        //     -> first rotate LEFT child to the left (makes it LL), then rotate RIGHT
        if (bf > 1 && p.productID > node->left->data.productID) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        // RL: right-heavy but new key is in RIGHT child's LEFT
        //     -> first rotate RIGHT child to the right (makes it RR), then rotate LEFT
        if (bf < -1 && p.productID < node->right->data.productID) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;   // already balanced
    }

    bool insert(const Product& p, int& comparisons) {
        comparisons = 0;
        bool inserted = false;
        root = insert(root, p, comparisons, inserted);
        return inserted;
    }

    const Product* search(int id, int& comparisons) const {
        comparisons = 0;
        AVLNode* current = root;
        while (current != nullptr) {
            comparisons++;
            if (id == current->data.productID) return &current->data;
            current = (id < current->data.productID) ? current->left : current->right;
        }
        return nullptr;
    }

    void inorder(AVLNode* node, int& printed, int limit) const {
        if (node == nullptr || printed >= limit) return;
        inorder(node->left, printed, limit);
        if (printed < limit) { cout << "   "; printProduct(node->data); printed++; }
        inorder(node->right, printed, limit);
    }
    void printFirst(int limit) const { int printed = 0; inorder(root, printed, limit); }
};

// =====================================================================
// 4. HASH TABLE - Open Addressing with Linear Probing
//    hash(id) = id % TABLE_SIZE ; on collision try next slot (+1, +2, ...)
//    TABLE_SIZE is PRIME (53) so keys spread more evenly over the slots.
// =====================================================================
const int TABLE_SIZE = 53;

class HashTable {
public:
    Product slot[TABLE_SIZE];
    bool    used[TABLE_SIZE];
    int     count = 0;
    int     totalCollisions = 0;

    HashTable() { for (int i = 0; i < TABLE_SIZE; i++) used[i] = false; }

    int hashFunction(int id) const { return id % TABLE_SIZE; }

    // 'collisions' = number of occupied slots we had to skip. Returns false if
    // duplicate or table full.
    bool insert(const Product& p, int& collisions) {
        collisions = 0;
        if (count >= TABLE_SIZE) return false;                 // table full
        int index = hashFunction(p.productID);
        for (int i = 0; i < TABLE_SIZE; i++) {
            if (!used[index]) {                                // empty slot found
                slot[index] = p; used[index] = true; count++;
                totalCollisions += collisions;
                return true;
            }
            if (slot[index].productID == p.productID) return false;   // duplicate
            collisions++;                                      // slot occupied -> COLLISION
            index = (index + 1) % TABLE_SIZE;                  // linear probe: next slot (wraps)
        }
        return false;
    }

    // 'probes' = number of slots examined. Search stops at the key or at an EMPTY slot
    // (an empty slot proves the key can't be further along the probe sequence).
    const Product* search(int id, int& probes) const {
        probes = 0;
        int index = hashFunction(id);
        for (int i = 0; i < TABLE_SIZE; i++) {
            probes++;
            if (!used[index]) return nullptr;                  // empty -> not present
            if (slot[index].productID == id) return &slot[index];
            index = (index + 1) % TABLE_SIZE;
        }
        return nullptr;                                        // table full, scanned everything
    }
    double loadFactor() const { return (double)count / TABLE_SIZE; }
};

// =====================================================================
// 5. DATASET: 40 unique products
//    First 20  : sequential IDs 1001-1020 (bulk batch added) -> makes BST skew
//    Last 20   : scattered IDs (individually added items)
// =====================================================================
vector<Product> loadDataset() {
    return {
        // ---- Batch 1: sequential IDs (simulates a bulk stock delivery) ----
        {1001, "USB-C Cable 1m",        "Accessories",  299.00},
        {1002, "Wireless Mouse",        "Peripherals",  699.00},
        {1003, "Mechanical Keyboard",   "Peripherals", 2499.00},
        {1004, "HDMI Cable 2m",         "Accessories",  349.00},
        {1005, "Laptop Stand",          "Accessories",  899.00},
        {1006, "Webcam 1080p",          "Peripherals", 1799.00},
        {1007, "Bluetooth Speaker",     "Audio",       1999.00},
        {1008, "Wired Earphones",       "Audio",        499.00},
        {1009, "Power Bank 10000mAh",   "Mobile",      1299.00},
        {1010, "Phone Case",            "Mobile",       249.00},
        {1011, "Screen Guard",          "Mobile",       199.00},
        {1012, "Memory Card 64GB",      "Storage",      599.00},
        {1013, "Pen Drive 32GB",        "Storage",      399.00},
        {1014, "External HDD 1TB",      "Storage",     3999.00},
        {1015, "Wi-Fi Router",          "Networking",  1599.00},
        {1016, "Ethernet Cable 5m",     "Networking",   279.00},
        {1017, "Smart Bulb",            "Home",         549.00},
        {1018, "Desk Lamp",             "Home",         799.00},
        {1019, "Surge Protector",       "Home",         999.00},
        {1020, "Laptop Sleeve 15in",    "Accessories",  649.00},
        // ---- Batch 2: scattered IDs ----
        {4587, "Gaming Headset",        "Audio",       2799.00},
        {2210, "Tablet 10in",           "Mobile",     14999.00},
        {7733, "Smart Watch",           "Wearables",   4999.00},
        {3018, "Fitness Band",          "Wearables",   1899.00},
        {9145, "27in Monitor",          "Displays",   12499.00},
        {5602, "Graphics Tablet",       "Peripherals", 5499.00},
        {1850, "Studio Microphone",     "Audio",       3299.00},
        {6391, "Portable SSD 512GB",    "Storage",     4599.00},
        {8274, "Action Camera",         "Cameras",     8999.00},
        {2967, "Tripod Stand",          "Cameras",      899.00},
        {4120, "Printer Ink Set",       "Printing",    1499.00},
        {7059, "Laser Printer",         "Printing",   11999.00},
        {3345, "Network Switch 8-port", "Networking",  2299.00},
        {9921, "Gaming Laptop",         "Computers",  74999.00},
        {5178, "Office Chair",          "Furniture",   6999.00},
        {6804, "Standing Desk",         "Furniture",  15999.00},
        {2503, "Noise-Cancel Headphone","Audio",       9999.00},
        {8416, "Drone Mini",            "Cameras",    24999.00},
        {1337, "Game Controller",       "Gaming",      2199.00},
        {7690, "VR Headset",            "Gaming",     29999.00}
    };
}

// =====================================================================
// 6. HELPERS: benchmark table + live demo
// =====================================================================
void line(char c = '-', int n = 72) { cout << string(n, c) << "\n"; }

void benchmark(const string& title, const vector<int>& ids,
               const BST& bst, const AVL& avl, const HashTable& ht) {
    cout << "\n" << title << "\n";
    line();
    cout << left << setw(12) << "Product ID" << setw(10) << "Found?"
         << right << setw(10) << "BST" << setw(10) << "AVL" << setw(12) << "Hash" << "\n";
    line();
    double sb = 0, sa = 0, sh = 0;
    for (int id : ids) {
        int cb, ca, ch;
        const Product* pb = bst.search(id, cb);
        const Product* pa = avl.search(id, ca);
        const Product* ph = ht.search(id, ch);
        sb += cb; sa += ca; sh += ch;
        cout << left << setw(12) << id << setw(10) << ((pb && pa && ph) ? "Yes" : "No")
             << right << setw(10) << cb << setw(10) << ca << setw(12) << ch << "\n";
    }
    line();
    cout << left << setw(22) << "AVERAGE comparisons" << right << setw(10) << fixed << setprecision(2)
         << sb / ids.size() << setw(10) << sa / ids.size() << setw(12) << sh / ids.size() << "\n";
}

void liveInsert(BST& bst, AVL& avl, HashTable& ht) {
    Product p;
    cout << "Enter new Product ID: "; cin >> p.productID;
    cin.ignore(10000, '\n');
    cout << "Enter product name  : "; getline(cin, p.productName);
    p.category = "Demo";
    cout << "Enter price         : "; cin >> p.price;

    int cb, ca, ch;
    int hb = bst.height(), ha = avl.height();
    int rotBefore = avl.rotationCount;
    bool okB = bst.insert(p, cb);
    bool okA = avl.insert(p, ca);
    bool okH = ht.insert(p, ch);

    if (!okB && !okA) { cout << ">> ID " << p.productID << " already exists - duplicate rejected.\n"; return; }
    cout << "\n--- Insertion result for ID " << p.productID << " ---\n";
    cout << "BST : " << cb << " comparisons | height " << hb << " -> " << bst.height() << "\n";
    cout << "AVL : " << ca << " comparisons | height " << ha << " -> " << avl.height()
         << " | rotations used: " << (avl.rotationCount - rotBefore) << "\n";
    if (okH)
        cout << "Hash: home slot " << ht.hashFunction(p.productID) << " | collisions (probes past home): "
             << ch << " | load factor " << fixed << setprecision(2) << ht.loadFactor() << "\n";
    else
        cout << "Hash: insertion failed (table full).\n";
}

void liveSearch(const BST& bst, const AVL& avl, const HashTable& ht) {
    int id; cout << "Enter Product ID to search: "; cin >> id;
    int cb, ca, ch;
    const Product* pb = bst.search(id, cb);
    const Product* pa = avl.search(id, ca);
    const Product* ph = ht.search(id, ch);
    cout << "\n--- Search result for ID " << id << " ---\n";
    if (pb) { cout << "Found: "; printProduct(*pb); } else cout << "NOT FOUND in the inventory.\n";
    cout << "BST comparisons : " << cb << "\n";
    cout << "AVL comparisons : " << ca << "\n";
    cout << "Hash probes     : " << ch << " (home slot " << ht.hashFunction(id) << ")\n";
    (void)pa; (void)ph;
}

// =====================================================================
// 7. MAIN: experiment + benchmark + interactive viva menu
// =====================================================================
int main() {
    BST bst; AVL avl; HashTable ht;
    vector<Product> data = loadDataset();

    // ---------------- PHASE 1: INSERTION EXPERIMENT ----------------
    cout << "========== PHASE 1: INSERTING " << data.size() << " PRODUCTS ==========\n";
    long bstCmp = 0, avlCmp = 0;
    for (const Product& p : data) {
        int c;
        bst.insert(p, c); bstCmp += c;
        avl.insert(p, c); avlCmp += c;
        ht.insert(p, c);
    }
    line('=');
    cout << "BST height (nodes)          : " << bst.height() << "\n";
    cout << "AVL height (nodes)          : " << avl.height() << "   (rotations performed: " << avl.rotationCount << ")\n";
    cout << "Total insertion comparisons : BST = " << bstCmp << ", AVL = " << avlCmp << "\n";
    cout << "Hash total collisions       : " << ht.totalCollisions << "  (load factor "
         << fixed << setprecision(2) << ht.loadFactor() << ", table size " << TABLE_SIZE << ")\n";

    cout << "\nFirst 10 products via BST in-order traversal (sorted by ID):\n";
    bst.printFirst(10);
    cout << "\nFirst 10 products via AVL in-order traversal (sorted by ID):\n";
    avl.printFirst(10);

    // ---------------- PHASE 2: BENCHMARK SEARCHES ----------------
    cout << "\n========== PHASE 2: BENCHMARK SEARCHES ==========";
    benchmark("5 SUCCESSFUL SEARCHES",   {1001, 1010, 1020, 6391, 9921}, bst, avl, ht);
    benchmark("5 UNSUCCESSFUL SEARCHES", {1021, 1500, 3000, 9999, 500},  bst, avl, ht);

    // ---------------- PHASE 3: LIVE VIVA MENU ----------------
    int choice = 0;
    while (choice != 5) {
        cout << "\n========== LIVE VIVA MENU ==========\n"
             << "1. Insert a new product\n"
             << "2. Search a product ID\n"
             << "3. Show current tree heights & hash load\n"
             << "4. Show first 10 products (in-order, AVL)\n"
             << "5. Exit\n"
             << "Choice: ";
        if (!(cin >> choice)) { cout << "Invalid input. Exiting.\n"; break; }
        switch (choice) {
            case 1: liveInsert(bst, avl, ht); break;
            case 2: liveSearch(bst, avl, ht); break;
            case 3:
                cout << "BST height = " << bst.height() << " | AVL height = " << avl.height()
                     << " | Hash load = " << fixed << setprecision(2) << ht.loadFactor()
                     << " | Hash collisions so far = " << ht.totalCollisions << "\n"; break;
            case 4: avl.printFirst(10); break;
            case 5: cout << "Goodbye!\n"; break;
            default: cout << "Please choose 1-5.\n";
        }
    }
    return 0;
}
