/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: [Student Name 1], [Student Name 2]
 * Roll Numbers: [Roll# 1], [Roll# 2]
 * Date: November 2025
 * Description: Friend request system using Array + Linked List
 *              Simplified version with linear search instead of hash table
 */

#pragma once

#include <string>

// Friend request status
enum RequestStatus {
    PENDING,
    ACCEPTED,
    REJECTED
};

// Friend request node
struct FriendRequest {
    int fromPlayerID;
    std::string fromUsername;
    int toPlayerID;
    std::string toUsername;
    RequestStatus status;
    FriendRequest* next;
    
    FriendRequest(int fromID, std::string fromName, int toID, std::string toName)
        : fromPlayerID(fromID), fromUsername(fromName), toPlayerID(toID), 
          toUsername(toName), status(PENDING), next(nullptr) {}
};

// Player with friends list
struct PlayerWithFriends {
    int playerID;
    std::string username;
    FriendRequest* friendsHead;      // Linked list of accepted friends
    FriendRequest* pendingHead;      // Linked list of pending requests
    
    PlayerWithFriends() : playerID(0), username(""), friendsHead(nullptr), pendingHead(nullptr) {}
};

// Friend System Manager
class FriendSystem {
private:
    static const int HASH_TABLE_SIZE = 101;  // Prime number for better distribution
    
    PlayerWithFriends players[100];  // Array of players
    int playerCount;
    int hashTable[HASH_TABLE_SIZE];  // Maps username hash -> player index (-1 = empty)
    
    // Hash function
    int hashFunction(const std::string& username);
    void insertIntoHashTable(const std::string& username, int playerIndex);
    int searchHashTable(const std::string& username);
    
    // Helper functions
    int findPlayerIndex(int playerID);
    int findPlayerIndexByUsername(const std::string& username);
    void loadFriendData();
    void saveFriendData();

public:
    FriendSystem();
    ~FriendSystem();
    
    // Main operations
    void addPlayer(int playerID, const std::string& username);
    bool sendFriendRequest(int fromPlayerID, const std::string& toUsername);
    bool acceptFriendRequest(int playerID, int fromPlayerID);
    bool rejectFriendRequest(int playerID, int fromPlayerID);
    void removeFriend(int playerID, int friendID);
    
    // View operations
    void viewFriendsList(int playerID);
    void viewPendingRequests(int playerID);
    int getFriendCount(int playerID);
    bool areFriends(int player1ID, int player2ID);
};
