/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
namespace sf { class RenderWindow; }
enum RequestStatus {
    PENDING,
    ACCEPTED,
    REJECTED
};
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
struct PlayerWithFriends {
    int playerID;
    std::string username;
    FriendRequest* friendsHead;      // Linked list of accepted friends
    FriendRequest* pendingHead;      // Linked list of pending requests
    PlayerWithFriends() : playerID(0), username(""), friendsHead(nullptr), pendingHead(nullptr) {}
};
class FriendSystem {
public:
    using FriendNode = FriendRequest;
private:
    static const int HASH_TABLE_SIZE = 101;  // Prime number for better distribution
    PlayerWithFriends players[100];  // Array of players
    int playerCount;
    int hashTable[HASH_TABLE_SIZE];  // Maps username hash -> player index (-1 = empty)
    int hashFunction(const std::string& username);
    void insertIntoHashTable(const std::string& username, int playerIndex);
    int searchHashTable(const std::string& username);
    int findPlayerIndexByUsername(const std::string& username);
    void loadFriendData();
    void saveFriendData();
public:
    FriendSystem();
    ~FriendSystem();
    int findPlayerIndex(int playerID);
    void addPlayer(int playerID, const std::string& username);
    bool sendFriendRequest(int fromPlayerID, const std::string& toUsername);
    bool acceptFriendRequest(int playerID, int fromPlayerID);
    bool rejectFriendRequest(int playerID, int fromPlayerID);
    void removeFriend(int playerID, int friendID);
    void viewFriendsList(int playerID);
    void viewPendingRequests(int playerID);
    int getFriendCount(int playerID);
    bool areFriends(int player1ID, int player2ID);
    FriendRequest* getPendingRequestsHead(int playerID);
    int getPendingRequestsCount(int playerID);
    friend bool selectFriendForMultiplayer(sf::RenderWindow*, FriendSystem*, int, int&, std::string&);
    friend void showFriendsMenu(sf::RenderWindow*, FriendSystem*, int);
};
