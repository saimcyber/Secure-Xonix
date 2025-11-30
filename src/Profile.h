#pragma once
#include <string>
#include <SFML/Graphics.hpp>
struct MatchEntry {
    int matchID;
    std::string opponent;
    int playerScore;
    int opponentScore;
    bool won;
    std::string date;
};
struct FriendNode {
    int friendPlayerID;
    std::string friendUsername;
    FriendNode* next;
    FriendNode(int id, std::string name) : friendPlayerID(id), friendUsername(name), next(nullptr) {}
};
class PlayerProfile {
private:
    int playerID;
    std::string username;
    int totalPoints;
    int matchesWon;
    int matchesLost;
    int currentLevel;
    int currentXP;
    int xpToNextLevel;
    FriendNode* friendsHead;  // Linked list of friends
    MatchEntry matchHistory[50];  // Array for match history
    int matchCount;
    std::string profileFile;
    void loadProfile();
    void saveProfile();
    void calculateXPRequirement();
    void checkLevelUp();
public:
    PlayerProfile();
    PlayerProfile(int id, std::string name);
    ~PlayerProfile();
    int getPlayerID() const { return playerID; }
    std::string getUsername() const { return username; }
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
    void addMatchResult(const std::string& opponent, int playerScore, int opponentScore, bool won);
    void addFriend(int friendID, const std::string& friendName);
    bool removeFriend(int friendID);
    std::string getLevelTitle() const;
    int getXPFromScore(int score) const;
    void displayProfile(sf::RenderWindow* window, sf::Font& font);
    void displayMatchHistory(sf::RenderWindow* window, sf::Font& font);
    void displayFriendsList(sf::RenderWindow* window, sf::Font& font);
    bool isFriend(int friendID);
    int getFriendCount();
};
