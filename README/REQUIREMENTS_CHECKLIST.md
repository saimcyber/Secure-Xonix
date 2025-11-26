# Project Requirements Checklist
**Analysis Date:** November 19, 2025  
**Submission Deadline:** November 28, 2025

---

## ✅ FULLY IMPLEMENTED (Ready for Demo)

### 1. Main Menu ✅
- [x] Single and Multiplayer mode selection
- [x] Start Game option
- [x] Select Level (difficulty selection)
- [x] Leaderboard view
- **Files:** `Menu.cpp`, `Menu.h`

### 2. End Menu ✅
- [x] Game ends properly
- [x] Shows final score
- [x] Highlights new high scores
- [x] Options: Restart, Main Menu, Exit
- **Files:** `Menu.cpp` (`showEndMenu`, `showMEndMenu`)

### 3. Login and Authentication ✅
- [x] Player registration with username/password
- [x] Email optional field
- [x] Username uniqueness validation
- [x] Password strength check (min 4 chars)
- [x] File handling for credentials
- [x] Registration timestamp stored
- [x] Unique player ID assigned
- [x] Login validation
- **Files:** `Authentication.cpp`, `Authentication.h`

### 4. Points System ✅ (NEEDS MINOR FIX)
- [x] 1 point per tile captured
- [x] >10 tiles = ×2 multiplier
- [x] Reward counter tracking
- [x] Threshold reduction after 3 occurrences (10→5)
- [x] ×4 multiplier after 5 occurrences
- [x] Power-up at score 50
- [x] Additional power-ups at 70, 100, 130, +30
- [x] Power-ups stack in inventory
- **Files:** `main.cpp` (lines 680-730 for single, 1080-1130 for multi)
- **Minor Issue:** Power-up logic might skip scores on large jumps (documented in CODE_ANALYSIS_AND_FIXES.md)

### 5. Player Profile ✅
- [x] Username display
- [x] Friends list (linked list)
- [x] Match history with win/loss
- [x] Total points tracking
- **Files:** `Profile.cpp`, `Profile.h`

### 6. Multiplayer Mode ✅
- [x] Both players on same board
- [x] P1: Arrow keys, P2: Q,G,A,X controls
- [x] Shared timer
- [x] Individual scores displayed
- [x] Individual power-ups displayed
- [x] Winner = highest score when both die
- **Files:** `main.cpp` (`MultiGame` function)
- **Note:** Collision rules implemented (lines 1190-1230)

### 7. Leaderboard ✅ PERFECT
- [x] Min-Heap implementation (fixed size 10)
- [x] Root = lowest score
- [x] Replace root if new score > min
- [x] Direct insertion if <10 players
- [x] Sorted display (descending)
- [x] Shows Player ID + Total Score
- [x] Syncs with Player Profile
- **Files:** `MinHeap.cpp`, `MinHeap.h`
- **Implementation:** Lines 80-120 (insert/update), 160-195 (display)

### 8. Priority Queue for Matchmaking ✅
- [x] Priority = total score (max heap)
- [x] Higher score = higher priority
- [x] Enqueue/dequeue operations
- [x] Can match when 2+ players
- **Files:** `PriorityQueue.cpp`, `PriorityQueue.h`
- **Status:** IMPLEMENTED but NOT USED in current menu flow

### 9. Game Room Queue ✅
- [x] FIFO queue using linked list
- [x] Automatic pairing capability
- [x] Display waiting players
- **Files:** `GameRoomQueue.cpp`, `GameRoomQueue.h`
- **Status:** IMPLEMENTED but NOT USED in current menu flow

### 10. Friend System ✅ (Partial)
- [x] Array list of players
- [x] Player ID per player
- [x] Linked list of friends per player
- [x] Send friend request
- [x] Accept/Reject requests
- [x] View friends (linked list traversal)
- **Files:** `FriendSystem.cpp`, `FriendSystem.h`

### 11. Save Game System ✅
- [x] GameState struct with timestamp
- [x] Player ID storage
- [x] Linked list of tiles (TileNode)
- [x] Serialization to file
- [x] Unique Save ID
- [x] Resume by Save ID
- [x] Grid reconstruction
- **Files:** `SaveGame.cpp`, `SaveGame.h`
- **Status:** Fully functional, NOT integrated into menu

---

## ⚠️ NEEDS MODIFICATION (Technical Debt)

### 1. Inventory System - BST → AVL Tree Required ⚠️
**Requirement:** "Inventory (AVL TREE) - Themes stored in AVL Tree"

**Current Status:**
- ✅ Has Binary Search Tree (BST)
- ❌ NOT self-balancing (no AVL rotations)
- ❌ Missing height tracking
- ❌ Missing balance factor calculation

**Files:** `Inventory.h`, `Inventory.cpp`

**What Needs to Change:**
```cpp
// Current (Inventory.h line 36-41):
struct ThemeNode {
    Theme theme;
    ThemeNode* left;
    ThemeNode* right;
    ThemeNode(Theme t) : theme(t), left(nullptr), right(nullptr) {}
};

// Required:
struct ThemeNode {
    Theme theme;
    ThemeNode* left;
    ThemeNode* right;
    int height;  // ADD THIS
    ThemeNode(Theme t) : theme(t), left(nullptr), right(nullptr), height(1) {}
};

// Need to add in ThemeInventory class:
- int getHeight(ThemeNode* node);
- int getBalanceFactor(ThemeNode* node);
- ThemeNode* rotateLeft(ThemeNode* node);
- ThemeNode* rotateRight(ThemeNode* node);
- Update insertHelper to balance after insertion
```

**Estimated Work:** 2-3 hours
**Priority:** HIGH - Explicitly required in rubric

---

### 2. Friend System - Hash Table Missing ⚠️
**Requirement:** "Hash table for mapping username → index"

**Current Status:**
- ✅ Has array of players
- ✅ Has linked list of friends
- ❌ Uses LINEAR SEARCH instead of hash table
- See `FriendSystem.h` line 8: "Simplified version with linear search instead of hash table"

**Files:** `FriendSystem.h`, `FriendSystem.cpp`

**What Needs to Change:**
```cpp
// Current (linear search):
int FriendSystem::findPlayerIndexByUsername(const string& username) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].username == username) return i;
    }
    return -1;
}

// Need to add:
class FriendSystem {
private:
    static const int HASH_TABLE_SIZE = 101;  // Prime number
    int hashTable[HASH_TABLE_SIZE];  // Maps hash → player index
    
    int hashFunction(const string& username) {
        // Simple hash: sum of ASCII values % table size
        int hash = 0;
        for (char c : username) {
            hash = (hash * 31 + c) % HASH_TABLE_SIZE;
        }
        return hash;
    }
    
    void insertIntoHashTable(const string& username, int index);
    int searchHashTable(const string& username);
};
```

**Estimated Work:** 3-4 hours (handle collisions with chaining)
**Priority:** HIGH - Explicitly required

---

### 3. File I/O Incomplete ⚠️
**Issue:** Friend System and Inventory save/load functions are stubs

**Friend System (FriendSystem.cpp lines 240-262):**
```cpp
void FriendSystem::loadFriendData() {
    // Simple implementation - just initialize empty
    // In full version, would load from file  ← NOT IMPLEMENTED
}
```

**Inventory System (Inventory.cpp lines 270-282):**
```cpp
void InventoryManager::loadInventoryData() {
    // Simple load implementation  ← NOT ACTUALLY LOADING
}
```

**Impact:** Data lost on program exit
**Priority:** MEDIUM - Will be noticed in demo
**Estimated Work:** 2 hours each

---

### 4. Multiplayer Collision Rules - Verify ⚠️
**Requirements:**
- If both collide while constructing → both die
- If P1 touches P2 constructing tile → P1 dies (vice versa)
- If P1 constructing touches P2 idle → P1 dies (vice versa)

**Current Implementation (main.cpp lines 1190-1230):**
```cpp
// Check collision with trails
if ((grid[p1newY][p1newX] == 3 && p1MoveCount > 0) || 
     grid[p1newY][p1newX] == 4) {
    p1Game = false;
}
```

**Status:** Partially implemented, needs verification for all 3 rules
**Priority:** MEDIUM - Core gameplay feature
**Estimated Work:** 1-2 hours testing and refinement

---

### 5. Matchmaking/Game Room Not Integrated ⚠️
**Issue:** 
- `MatchmakingPriorityQueue` and `GameRoomQueue` fully implemented
- BUT never called from menu system
- No way for players to actually use matchmaking

**Current:** Multiplayer uses same player twice (Menu.cpp line 476)
```cpp
MultiGame(window, difficulty, g_currentPlayerID, g_currentUsername, 
          g_currentPlayerID, g_currentUsername);  // Same player!
```

**Priority:** LOW - Can demo with manual player selection
**Estimated Work:** 4-5 hours to build full matchmaking UI

---

## ✅ BONUS FEATURES ALREADY IMPLEMENTED

### Save Game System ✅
- Complete implementation with linked list of tiles
- Not integrated into menu but fully functional
- Can be demoed separately

### Pause Menu ✅
- Full pause functionality
- Save state during pause
- Resume/Restart/Exit options

### Multiple Enemy Movement Patterns ✅
- Linear movement
- Zigzag pattern
- Drifting spiral pattern
- Pattern switching at 30 seconds

### Difficulty Levels ✅
- Easy (2 enemies)
- Medium (4 enemies)
- Hard (6 enemies)
- Continuous (escalating enemies)

---

## 📋 IMMEDIATE ACTION ITEMS (Before Nov 28)

### CRITICAL (Must Complete):
1. **Convert Inventory BST to AVL Tree** (HIGH priority)
   - Add height field to ThemeNode
   - Implement rotation functions
   - Update insert to balance tree
   - Estimated: 3 hours

2. **Add Hash Table to Friend System** (HIGH priority)
   - Implement hash function
   - Add hash table array
   - Handle collisions (chaining recommended)
   - Update search to use hash table
   - Estimated: 4 hours

3. **Fix File I/O for Persistence** (MEDIUM priority)
   - Implement Friend System save/load
   - Implement Inventory save/load
   - Test data persistence
   - Estimated: 4 hours

### RECOMMENDED (Should Complete):
4. **Verify Multiplayer Collision Rules** (MEDIUM priority)
   - Test all 3 collision scenarios
   - Add comments explaining logic
   - Estimated: 2 hours

5. **Fix Power-Up Award Logic** (LOW priority)
   - Handle large score jumps
   - Award all missed power-ups
   - Estimated: 1 hour

### OPTIONAL (Nice to Have):
6. **Integrate Matchmaking System**
   - Add menu option for online matchmaking
   - Connect to MatchmakingPriorityQueue
   - Estimated: 5 hours

7. **Integrate Save Game**
   - Add "Load Game" to menu
   - Add "Save Game" to pause menu
   - Estimated: 2 hours

---

## 📊 COMPLETION STATUS

### Data Structures Implementation:
- [x] Arrays - Used throughout
- [x] Linked Lists - Friends, Match History, Save Game tiles
- [x] Binary Search Tree - Inventory (needs → AVL)
- [ ] **AVL Tree - MISSING (required for Inventory)**
- [ ] **Hash Table - MISSING (required for Friend System)**
- [x] Min-Heap - Leaderboard (perfect implementation)
- [x] Priority Queue (Max-Heap) - Matchmaking
- [x] Queue (Linked List) - Game Room
- [x] File Handling - All systems

### Features Implementation:
- **Authentication:** 100% ✅
- **Main Menu:** 100% ✅
- **Game Modes:** 100% ✅
- **Points System:** 95% ✅ (minor bug)
- **Leaderboard:** 100% ✅
- **Profile:** 100% ✅
- **Multiplayer:** 90% ✅ (verify collisions)
- **Friend System:** 80% ⚠️ (missing hash table)
- **Inventory:** 70% ⚠️ (BST not AVL)
- **Save Game:** 100% ✅ (not integrated)
- **Matchmaking:** 100% ✅ (not integrated)

### Overall Completion: **85%**

---

## 🎯 RECOMMENDED WORK PLAN

### Day 1-2 (Nov 20-21): Critical Features
- [ ] Implement AVL Tree for Inventory (3 hours)
- [ ] Test AVL rotations and balancing (1 hour)
- [ ] Implement Hash Table for Friends (4 hours)
- [ ] Test hash table with collisions (1 hour)

### Day 3 (Nov 22): File Persistence
- [ ] Implement Friend System file I/O (2 hours)
- [ ] Implement Inventory file I/O (2 hours)
- [ ] Test all persistence across restarts (2 hours)

### Day 4 (Nov 23): Testing & Bug Fixes
- [ ] Verify multiplayer collision rules (2 hours)
- [ ] Fix power-up award logic (1 hour)
- [ ] Overall system testing (3 hours)

### Day 5-6 (Nov 24-25): Optional Features
- [ ] Integrate Save/Load game to menu (2 hours)
- [ ] Add visual polish (2 hours)
- [ ] Final testing (2 hours)

### Day 7 (Nov 26): Report Writing
- [ ] Write report sections
- [ ] Create workflow diagrams
- [ ] Take screenshots
- [ ] Review and finalize

### Day 8 (Nov 27): Final Submission Prep
- [ ] Code cleanup and comments
- [ ] Build executable
- [ ] Create ZIP package
- [ ] Submit before Nov 28

---

## 🚨 DEMO PREPARATION

### Be Ready to Explain:
1. **AVL Tree Implementation** - How rotations work, why balanced
2. **Hash Table** - Hash function choice, collision handling
3. **Min-Heap Leaderboard** - Why min-heap for top 10
4. **Friend System** - Why linked list for friends
5. **Save Game** - Why linked list for tiles

### Be Ready to Demo:
1. Register new player
2. Login
3. Play game and show points calculation
4. Show leaderboard updates
5. Send/accept friend request
6. View profile with match history
7. Play multiplayer mode
8. Show theme inventory
9. (Optional) Save/load game

---

## 📝 CODE QUALITY CHECKLIST

- [ ] All files have header comments with names and roll numbers
- [ ] Functions have meaningful names
- [ ] Complex logic has explanatory comments
- [ ] No magic numbers (use constants)
- [ ] Proper error handling
- [ ] Memory cleanup (destructors)
- [ ] No memory leaks
- [ ] Consistent code formatting
- [ ] No unused code/variables

---

**Last Updated:** November 19, 2025  
**Status:** 85% Complete - 15% Critical Work Remaining  
**Risk Level:** MEDIUM - Achievable with focused effort
