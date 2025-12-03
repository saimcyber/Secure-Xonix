/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#include "FriendSystem.h"
#include "Profile.h"
#include <iostream>
#include <fstream>
using namespace std;
FriendSystem::FriendSystem() {
    playerCount = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = -1;
    }
    loadFriendData();
}
FriendSystem::~FriendSystem() {
    saveFriendData();
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
int FriendSystem::findPlayerIndex(int playerID) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].playerID == playerID) {
            return i;
        }
    }
    return -1;
}
int FriendSystem::hashFunction(const string& username) {
    int hash = 0;
    for (char c : username) {
        hash = (hash * 31 + c) % HASH_TABLE_SIZE;
    }
    return hash;
}
void FriendSystem::insertIntoHashTable(const string& username, int playerIndex) {
    int hash = hashFunction(username);
    int originalHash = hash;
    while (hashTable[hash] != -1) {
        hash = (hash + 1) % HASH_TABLE_SIZE;
        if (hash == originalHash) {
            return;
        }
    }
    hashTable[hash] = playerIndex;
}
int FriendSystem::searchHashTable(const string& username) {
    int hash = hashFunction(username);
    int originalHash = hash;
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
int FriendSystem::findPlayerIndexByUsername(const string& username) {
    return searchHashTable(username);
}
void FriendSystem::addPlayer(int playerID, const string& username) {
    int index = findPlayerIndex(playerID);
    if (index != -1) {
        return; // Player already exists
    }
    if (playerCount >= 100) {
        cout << "Error: Maximum players reached!" << endl;
        return;
    }
    players[playerCount].playerID = playerID;
    players[playerCount].username = username;
    players[playerCount].friendsHead = nullptr;
    players[playerCount].pendingHead = nullptr;
    insertIntoHashTable(username, playerCount);
    playerCount++;
}
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
    if (areFriends(fromPlayerID, players[toIndex].playerID)) {
        cout << "Already friends!" << endl;
        return false;
    }
    FriendRequest* current = players[toIndex].pendingHead;
    while (current) {
        if (current->fromPlayerID == fromPlayerID) {
            cout << "Request already sent!" << endl;
            return false;
        }
        current = current->next;
    }
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
bool FriendSystem::acceptFriendRequest(int playerID, int fromPlayerID) {
    int playerIndex = findPlayerIndex(playerID);
    int fromIndex = findPlayerIndex(fromPlayerID);
    if (playerIndex == -1 || fromIndex == -1) return false;
    FriendRequest* current = players[playerIndex].pendingHead;
    FriendRequest* prev = nullptr;
    while (current) {
        if (current->fromPlayerID == fromPlayerID) {
            if (prev) {
                prev->next = current->next;
            } else {
                players[playerIndex].pendingHead = current->next;
            }
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
            
            // Sync with PlayerProfile system
            {
                PlayerProfile playerProfile(playerID, players[playerIndex].username);
                playerProfile.addFriend(fromPlayerID, players[fromIndex].username);
                // Profile saves on destruction (when going out of scope)
            }
            {
                PlayerProfile friendProfile(fromPlayerID, players[fromIndex].username);
                friendProfile.addFriend(playerID, players[playerIndex].username);
                // Profile saves on destruction (when going out of scope)
            }
            
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
void FriendSystem::removeFriend(int playerID, int friendID) {
    int playerIndex = findPlayerIndex(playerID);
    int friendIndex = findPlayerIndex(friendID);
    if (playerIndex == -1 || friendIndex == -1) return;
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
    
    // Sync with PlayerProfile system
    {
        PlayerProfile playerProfile(playerID, players[playerIndex].username);
        playerProfile.removeFriend(friendID);
        // Profile saves on destruction
    }
    {
        PlayerProfile friendProfile(friendID, players[friendIndex].username);
        friendProfile.removeFriend(playerID);
        // Profile saves on destruction
    }
    
    saveFriendData();  // Save immediately after removal
    
    cout << "Friend removed!" << endl;
}
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
void FriendSystem::loadFriendData() {
    ifstream file("data/friends.txt");
    if (!file.is_open()) {
        cout << "No existing friends data. Starting fresh." << endl;
        return;
    }
    int savedPlayerCount;
    file >> savedPlayerCount;
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
        if (findPlayerIndex(pID) == -1) {
            addPlayer(pID, uname);
        }
        int playerIdx = findPlayerIndex(pID);
        if (playerIdx == -1) continue;
        int friendCount;
        file >> friendCount;
        if (file.fail() || friendCount < 0) break;  // Stop if read fails
        for (int j = 0; j < friendCount; j++) {
            int fromID, toID;
            string fromName, toName;
            file >> fromID >> toID;
            if (file.fail()) break;
            file.ignore();
            getline(file, fromName, ',');
            getline(file, toName);
            if (file.fail()) break;
            FriendRequest* newFriend = new FriendRequest(fromID, fromName, toID, toName);
            newFriend->status = ACCEPTED;
            newFriend->next = players[playerIdx].friendsHead;
            players[playerIdx].friendsHead = newFriend;
        }
        int pendingCount;
        file >> pendingCount;
        if (file.fail() || pendingCount < 0) break;  // Stop if read fails
        for (int j = 0; j < pendingCount; j++) {
            int fromID, toID;
            string fromName, toName;
            file >> fromID >> toID;
            if (file.fail()) break;
            file.ignore();
            getline(file, fromName, ',');
            getline(file, toName);
            if (file.fail()) break;
            FriendRequest* newRequest = new FriendRequest(fromID, fromName, toID, toName);
            newRequest->status = PENDING;
            newRequest->next = players[playerIdx].pendingHead;
            players[playerIdx].pendingHead = newRequest;
        }
    }
    file.close();
    cout << "Loaded friend data for " << savedPlayerCount << " players." << endl;
}
void FriendSystem::saveFriendData() {
    ofstream file("data/friends.txt");
    if (!file.is_open()) {
        cout << "Error: Could not save friend data!" << endl;
        return;
    }
    file << playerCount << "\n";
    for (int i = 0; i < playerCount; i++) {
        file << players[i].playerID << " " << players[i].username << "\n";
        int friendCount = 0;
        FriendRequest* fr = players[i].friendsHead;
        while (fr) {
            friendCount++;
            fr = fr->next;
        }
        file << friendCount << "\n";
        fr = players[i].friendsHead;
        while (fr) {
            file << fr->fromPlayerID << " " << fr->toPlayerID << " ";
            file << fr->fromUsername << "," << fr->toUsername << "\n";
            fr = fr->next;
        }
        int pendingCount = 0;
        FriendRequest* pr = players[i].pendingHead;
        while (pr) {
            pendingCount++;
            pr = pr->next;
        }
        file << pendingCount << "\n";
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
FriendRequest* FriendSystem::getPendingRequestsHead(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return nullptr;
    return players[index].pendingHead;
}
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
