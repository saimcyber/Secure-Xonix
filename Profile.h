/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Player profile system header - stores player stats and match history
 */

#pragma once

#include <string>
#include <SFML/Graphics.hpp>

// Match history entry (simple array-based implementation)
struct MatchEntry {
    int matchID;
    std::string opponent;
    int playerScore;
    int opponentScore;
    bool won;
    std::string date;
};

// Simple friend list node (linked list)
struct FriendNode {
    int friendPlayerID;
    std::string friendUsername;
    FriendNode* next;
    
    FriendNode(int id, std::string name) : friendPlayerID(id), friendUsername(name), next(nullptr) {}
};

// Player profile class
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
    
    // Getters
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
    
    // Setters
    void addPoints(int points);
    void addXP(int xp);
    void addMatchResult(const std::string& opponent, int playerScore, int opponentScore, bool won);
    void addFriend(int friendID, const std::string& friendName);
    bool removeFriend(int friendID);
    
    // Level system
    std::string getLevelTitle() const;
    int getXPFromScore(int score) const;
    
    // Display functions
    void displayProfile(sf::RenderWindow* window, sf::Font& font);
    void displayMatchHistory(sf::RenderWindow* window, sf::Font& font);
    void displayFriendsList(sf::RenderWindow* window, sf::Font& font);
    
    // Friend list helpers
    bool isFriend(int friendID);
    int getFriendCount();
};
