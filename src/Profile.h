/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
#include <SFML/Graphics.hpp>
using namespace std;
struct MatchEntry {
    int matchID;
    string opponent;
    int playerScore;
    int opponentScore;
    bool won;
    string date;
};
struct FriendNode {
    int friendPlayerID;
    string friendUsername;
    FriendNode* next;
    FriendNode(int id, string name) : friendPlayerID(id), friendUsername(name), next(nullptr) {}
};
class PlayerProfile {
private:
    int playerID;
    string username;
    int totalPoints;
    int matchesWon;
    int matchesLost;
    int currentLevel;
    int currentXP;
    int xpToNextLevel;
    FriendNode* friendsHead;  // Linked list of friends
    MatchEntry matchHistory[50];  // Array for match history
    int matchCount;
    string profileFile;
    void loadProfile();
    void saveProfile();
    void calculateXPRequirement();
    void checkLevelUp();
public:
    PlayerProfile();
    PlayerProfile(int id, string name);
    ~PlayerProfile();
    int getPlayerID() const { return playerID; }
    string getUsername() const { return username; }
    int getTotalPoints() const { return totalPoints; }
    int getMatchesWon() const { return matchesWon; }
    int getMatchesLost() const { return matchesLost; }
    int getMatchCount() const { return matchCount; }
    int getCurrentLevel() const { return currentLevel; }
    int getCurrentXP() const { return currentXP; }
    int getXPToNextLevel() const { return xpToNextLevel; }
    float getXPProgress() const { return (float)currentXP / (float)xpToNextLevel; }
    void addPoints(int points);
    void addXP(int xp);
    void addMatchResult(const string& opponent, int playerScore, int opponentScore, bool won);
    void addFriend(int friendID, const string& friendName);
    bool removeFriend(int friendID);
    string getLevelTitle() const;
    int getXPFromScore(int score) const;
    void displayProfile(sf::RenderWindow* window, sf::Font& font);
    void displayMatchHistory(sf::RenderWindow* window, sf::Font& font);
    void displayFriendsList(sf::RenderWindow* window, sf::Font& font);
    bool isFriend(int friendID);
    int getFriendCount();
};
