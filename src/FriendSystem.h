/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
using namespace std;
namespace sf
{
    class RenderWindow;
}
// Request status constants
const string REQUEST_PENDING = "PENDING";
const string REQUEST_ACCEPTED = "ACCEPTED";
const string REQUEST_REJECTED = "REJECTED";

struct FriendRequest
{
    int fromPlayerID;
    string fromUsername;
    int toPlayerID;
    string toUsername;
    string status;
    FriendRequest *next;
    FriendRequest(int fromID, string fromName, int toID, string toName)
        : fromPlayerID(fromID), fromUsername(fromName), toPlayerID(toID),
          toUsername(toName), status(REQUEST_PENDING), next(nullptr) {}
};
struct PlayerWithFriends
{
    int playerID;
    string username;
    FriendRequest *friendsHead; // Linked list of accepted friends
    FriendRequest *pendingHead; // Linked list of pending requests
    PlayerWithFriends() : playerID(0), username(""), friendsHead(nullptr), pendingHead(nullptr) {}
};
class FriendSystem
{
public:
    using FriendNode = FriendRequest;

private:
    static const int HASH_TABLE_SIZE = 101; // Prime number for better distribution
    PlayerWithFriends players[100];         // Array of players
    int playerCount;
    int hashTable[HASH_TABLE_SIZE]; // Maps username hash -> player index (-1 = empty)
    int hashFunction(const string &username);
    void insertIntoHashTable(const string &username, int playerIndex);
    int searchHashTable(const string &username);
    int findPlayerIndexByUsername(const string &username);
    void loadFriendData();
    void saveFriendData();

public:
    FriendSystem();
    ~FriendSystem();
    int findPlayerIndex(int playerID);
    void addPlayer(int playerID, const string &username);
    bool sendFriendRequest(int fromPlayerID, const string &toUsername);
    bool acceptFriendRequest(int playerID, int fromPlayerID);
    bool rejectFriendRequest(int playerID, int fromPlayerID);
    void removeFriend(int playerID, int friendID);
    void viewFriendsList(int playerID);
    void viewPendingRequests(int playerID);
    int getFriendCount(int playerID);
    bool areFriends(int player1ID, int player2ID);
    FriendRequest *getPendingRequestsHead(int playerID);
    int getPendingRequestsCount(int playerID);
    friend bool selectFriendForMultiplayer(sf::RenderWindow *, FriendSystem *, int, int &, string &);
    friend void showFriendsMenu(sf::RenderWindow *, FriendSystem *, int);
};
