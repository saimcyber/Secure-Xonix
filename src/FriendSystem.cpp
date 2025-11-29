/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Friend system implementation
 */

#include "FriendSystem.h"
#include <iostream>
#include <fstream>

using namespace std;

// Constructor
FriendSystem::FriendSystem() {
    playerCount = 0;
    
    // Initialize hash table to -1 (empty)
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = -1;
    }
    
    loadFriendData();
}

// Destructor
FriendSystem::~FriendSystem() {
    saveFriendData();
    
    // Clean up all linked lists
    for (int i = 0; i < playerCount; i++) {
        FriendRequest* current = players[i].friendsHead;
        while (current) {
            FriendRequest* temp = current;
            current = current->next;
            delete temp;
        }
        
        current = players[i].pendingHead;
        while (current) {
            FriendRequest* temp = current;
            current = current->next;
            delete temp;
        }
    }
}

// Find player index by ID (linear search)
int FriendSystem::findPlayerIndex(int playerID) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].playerID == playerID) {
            return i;
        }
    }
    return -1;
}

// ===== Hash Table Implementation =====

// Simple hash function (sum of ASCII values mod table size)
int FriendSystem::hashFunction(const string& username) {
    int hash = 0;
    for (char c : username) {
        hash = (hash * 31 + c) % HASH_TABLE_SIZE;
    }
    return hash;
}

// Insert username into hash table (with linear probing for collisions)
void FriendSystem::insertIntoHashTable(const string& username, int playerIndex) {
    int hash = hashFunction(username);
    int originalHash = hash;
    
    // Linear probing: find next empty slot
    while (hashTable[hash] != -1) {
        hash = (hash + 1) % HASH_TABLE_SIZE;
        if (hash == originalHash) {
            // Table is full (shouldn't happen with 100 players and 101 slots)
            return;
        }
    }
    
    hashTable[hash] = playerIndex;
}

// Search for username in hash table (returns player index or -1)
int FriendSystem::searchHashTable(const string& username) {
    int hash = hashFunction(username);
    int originalHash = hash;
    
    // Linear probing: search until we find the username or an empty slot
    while (hashTable[hash] != -1) {
        if (players[hashTable[hash]].username == username) {
            return hashTable[hash];  // Found it!
        }
        hash = (hash + 1) % HASH_TABLE_SIZE;
        if (hash == originalHash) {
            break;  // Searched entire table
        }
    }
    
    return -1;  // Not found
}

// Find player index by username (now uses hash table - O(1) average case)
int FriendSystem::findPlayerIndexByUsername(const string& username) {
    return searchHashTable(username);
}

// Add player to the system
void FriendSystem::addPlayer(int playerID, const string& username) {
    cout << "addPlayer called for ID=" << playerID << " username=" << username << endl;
    cout.flush();
    
    int index = findPlayerIndex(playerID);
    cout << "findPlayerIndex returned: " << index << endl;
    cout.flush();
    
    if (index != -1) {
        cout << "Player already exists, returning" << endl;
        cout.flush();
        return; // Already exists
    }
    
    if (playerCount >= 100) {
        cout << "Maximum players reached!" << endl;
        return;
    }
    
    cout << "Adding player at index " << playerCount << endl;
    cout.flush();
    
    players[playerCount].playerID = playerID;
    players[playerCount].username = username;
    players[playerCount].friendsHead = nullptr;
    players[playerCount].pendingHead = nullptr;
    
    cout << "Inserting into hash table..." << endl;
    cout.flush();
    
    // Insert into hash table for fast lookup
    insertIntoHashTable(username, playerCount);
    
    cout << "Hash table insert complete" << endl;
    cout.flush();
    
    playerCount++;
    
    cout << "Player added successfully, new count: " << playerCount << endl;
    cout.flush();
}

// Send friend request
bool FriendSystem::sendFriendRequest(int fromPlayerID, const string& toUsername) {
    int fromIndex = findPlayerIndex(fromPlayerID);
    int toIndex = findPlayerIndexByUsername(toUsername);
    
    if (fromIndex == -1 || toIndex == -1) {
        cout << "Player not found!" << endl;
        return false;
    }
    
    if (fromPlayerID == players[toIndex].playerID) {
        cout << "Cannot send friend request to yourself!" << endl;
        return false;
    }
    
    // Check if already friends
    if (areFriends(fromPlayerID, players[toIndex].playerID)) {
        cout << "Already friends!" << endl;
        return false;
    }
    
    // Check if request already pending
    FriendRequest* current = players[toIndex].pendingHead;
    while (current) {
        if (current->fromPlayerID == fromPlayerID) {
            cout << "Request already sent!" << endl;
            return false;
        }
        current = current->next;
    }
    
    // Add to pending requests
    FriendRequest* newRequest = new FriendRequest(
        fromPlayerID, players[fromIndex].username,
        players[toIndex].playerID, players[toIndex].username
    );
    
    newRequest->next = players[toIndex].pendingHead;
    players[toIndex].pendingHead = newRequest;
    
    saveFriendData();  // Save immediately after sending request
    cout << "Friend request sent to " << toUsername << "!" << endl;
    return true;
}

// Accept friend request
bool FriendSystem::acceptFriendRequest(int playerID, int fromPlayerID) {
    int playerIndex = findPlayerIndex(playerID);
    int fromIndex = findPlayerIndex(fromPlayerID);
    
    if (playerIndex == -1 || fromIndex == -1) return false;
    
    // Find and remove from pending list
    FriendRequest* current = players[playerIndex].pendingHead;
    FriendRequest* prev = nullptr;
    
    while (current) {
        if (current->fromPlayerID == fromPlayerID) {
            // Remove from pending
            if (prev) {
                prev->next = current->next;
            } else {
                players[playerIndex].pendingHead = current->next;
            }
            
            // Add to both players' friends list
            FriendRequest* friend1 = new FriendRequest(
                fromPlayerID, players[fromIndex].username, playerID, players[playerIndex].username
            );
            friend1->status = ACCEPTED;
            friend1->next = players[playerIndex].friendsHead;
            players[playerIndex].friendsHead = friend1;
            
            FriendRequest* friend2 = new FriendRequest(
                playerID, players[playerIndex].username, fromPlayerID, players[fromIndex].username
            );
            friend2->status = ACCEPTED;
            friend2->next = players[fromIndex].friendsHead;
            players[fromIndex].friendsHead = friend2;
            
            delete current;
            saveFriendData();  // Save immediately after accepting
            cout << "Friend request accepted!" << endl;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    cout << "No pending request found!" << endl;
    return false;
}

// Reject friend request
bool FriendSystem::rejectFriendRequest(int playerID, int fromPlayerID) {
    int playerIndex = findPlayerIndex(playerID);
    if (playerIndex == -1) return false;
    
    FriendRequest* current = players[playerIndex].pendingHead;
    FriendRequest* prev = nullptr;
    
    while (current) {
        if (current->fromPlayerID == fromPlayerID) {
            if (prev) {
                prev->next = current->next;
            } else {
                players[playerIndex].pendingHead = current->next;
            }
            delete current;
            saveFriendData();  // Save immediately after reject
            cout << "Friend request rejected!" << endl;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

// Remove friend
void FriendSystem::removeFriend(int playerID, int friendID) {
    int playerIndex = findPlayerIndex(playerID);
    int friendIndex = findPlayerIndex(friendID);
    
    if (playerIndex == -1 || friendIndex == -1) return;
    
    // Remove from player's friends list
    FriendRequest* current = players[playerIndex].friendsHead;
    FriendRequest* prev = nullptr;
    
    while (current) {
        if (current->fromPlayerID == friendID || current->toPlayerID == friendID) {
            if (prev) {
                prev->next = current->next;
            } else {
                players[playerIndex].friendsHead = current->next;
            }
            delete current;
            break;
        }
        prev = current;
        current = current->next;
    }
    
    // Remove from friend's friends list
    current = players[friendIndex].friendsHead;
    prev = nullptr;
    
    while (current) {
        if (current->fromPlayerID == playerID || current->toPlayerID == playerID) {
            if (prev) {
                prev->next = current->next;
            } else {
                players[friendIndex].friendsHead = current->next;
            }
            delete current;
            break;
        }
        prev = current;
        current = current->next;
    }
    
    saveFriendData();  // Save immediately after removal
    cout << "Friend removed!" << endl;
}

// View friends list
void FriendSystem::viewFriendsList(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) {
        cout << "Player not found!" << endl;
        return;
    }
    
    cout << "\n=== Friends List ===" << endl;
    FriendRequest* current = players[index].friendsHead;
    int count = 0;
    
    while (current) {
        count++;
        string friendName = (current->fromPlayerID == playerID) ? current->toUsername : current->fromUsername;
        cout << count << ". " << friendName << endl;
        current = current->next;
    }
    
    if (count == 0) {
        cout << "No friends yet." << endl;
    }
}

// View pending requests
void FriendSystem::viewPendingRequests(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return;
    
    cout << "\n=== Pending Friend Requests ===" << endl;
    FriendRequest* current = players[index].pendingHead;
    int count = 0;
    
    while (current) {
        count++;
        cout << count << ". From: " << current->fromUsername << " (ID: " << current->fromPlayerID << ")" << endl;
        current = current->next;
    }
    
    if (count == 0) {
        cout << "No pending requests." << endl;
    }
}

// Get friend count
int FriendSystem::getFriendCount(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return 0;
    
    int count = 0;
    FriendRequest* current = players[index].friendsHead;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

// Check if two players are friends
bool FriendSystem::areFriends(int player1ID, int player2ID) {
    int index = findPlayerIndex(player1ID);
    if (index == -1) return false;
    
    FriendRequest* current = players[index].friendsHead;
    while (current) {
        if (current->fromPlayerID == player2ID || current->toPlayerID == player2ID) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Load friend data from file
void FriendSystem::loadFriendData() {
    ifstream file("data/friends.txt");
    if (!file.is_open()) {
        cout << "No existing friends data. Starting fresh." << endl;
        return;
    }
    
    int savedPlayerCount;
    file >> savedPlayerCount;
    
    // Check if file read was successful
    if (file.fail() || savedPlayerCount < 0 || savedPlayerCount > 100) {
        cout << "Invalid or empty friends data file. Starting fresh." << endl;
        file.close();
        return;
    }
    
    for (int i = 0; i < savedPlayerCount && i < 100; i++) {
        int pID;
        string uname;
        file >> pID;
        
        if (file.fail()) break;  // Stop if read fails
        
        file.ignore();
        getline(file, uname);
        
        if (file.fail()) break;  // Stop if read fails
        
        // Add player if not already exists
        if (findPlayerIndex(pID) == -1) {
            addPlayer(pID, uname);
        }
        
        int playerIdx = findPlayerIndex(pID);
        if (playerIdx == -1) continue;
        
        // Load friends count
        int friendCount;
        file >> friendCount;
        
        if (file.fail() || friendCount < 0) break;  // Stop if read fails
        
        // Load each friend
        for (int j = 0; j < friendCount; j++) {
            int fromID, toID;
            string fromName, toName;
            file >> fromID >> toID;
            
            if (file.fail()) break;
            
            file.ignore();
            getline(file, fromName, ',');
            getline(file, toName);
            
            if (file.fail()) break;
            
            // Create friend entry
            FriendRequest* newFriend = new FriendRequest(fromID, fromName, toID, toName);
            newFriend->status = ACCEPTED;
            newFriend->next = players[playerIdx].friendsHead;
            players[playerIdx].friendsHead = newFriend;
        }
        
        // Load pending requests count
        int pendingCount;
        file >> pendingCount;
        
        if (file.fail() || pendingCount < 0) break;  // Stop if read fails
        
        // Load each pending request
        for (int j = 0; j < pendingCount; j++) {
            int fromID, toID;
            string fromName, toName;
            file >> fromID >> toID;
            
            if (file.fail()) break;
            
            file.ignore();
            getline(file, fromName, ',');
            getline(file, toName);
            
            if (file.fail()) break;
            
            // Create pending request
            FriendRequest* newRequest = new FriendRequest(fromID, fromName, toID, toName);
            newRequest->status = PENDING;
            newRequest->next = players[playerIdx].pendingHead;
            players[playerIdx].pendingHead = newRequest;
        }
    }
    
    file.close();
    cout << "Loaded friend data for " << savedPlayerCount << " players." << endl;
}

// Save friend data to file
void FriendSystem::saveFriendData() {
    ofstream file("data/friends.txt");
    if (!file.is_open()) {
        cout << "Error: Could not save friend data!" << endl;
        return;
    }
    
    // Save player count
    file << playerCount << "\n";
    
    // Save each player's data
    for (int i = 0; i < playerCount; i++) {
        file << players[i].playerID << " " << players[i].username << "\n";
        
        // Count friends
        int friendCount = 0;
        FriendRequest* fr = players[i].friendsHead;
        while (fr) {
            friendCount++;
            fr = fr->next;
        }
        
        // Save friend count
        file << friendCount << "\n";
        
        // Save each friend
        fr = players[i].friendsHead;
        while (fr) {
            file << fr->fromPlayerID << " " << fr->toPlayerID << " ";
            file << fr->fromUsername << "," << fr->toUsername << "\n";
            fr = fr->next;
        }
        
        // Count pending requests
        int pendingCount = 0;
        FriendRequest* pr = players[i].pendingHead;
        while (pr) {
            pendingCount++;
            pr = pr->next;
        }
        
        // Save pending count
        file << pendingCount << "\n";
        
        // Save each pending request
        pr = players[i].pendingHead;
        while (pr) {
            file << pr->fromPlayerID << " " << pr->toPlayerID << " ";
            file << pr->fromUsername << "," << pr->toUsername << "\n";
            pr = pr->next;
        }
    }
    
    file.close();
    cout << "Friend data saved successfully." << endl;
}

// Get pending requests head for a player
FriendRequest* FriendSystem::getPendingRequestsHead(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return nullptr;
    return players[index].pendingHead;
}

// Get count of pending requests for a player
int FriendSystem::getPendingRequestsCount(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return 0;
    
    int count = 0;
    FriendRequest* current = players[index].pendingHead;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}
