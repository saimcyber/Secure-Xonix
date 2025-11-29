/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Authentication system header - handles player login and registration
 */

#pragma once

#include <string>
#include <SFML/Graphics.hpp>

// Maximum number of registered players
const int MAX_PLAYERS = 100;

// Player account structure
struct PlayerAccount {
    std::string username;
    std::string password;
    std::string email;
    int playerID;
    std::string registrationDate;
    bool isActive;
};

// Authentication class
class Authentication {
private:
    PlayerAccount players[MAX_PLAYERS];
    int playerCount;
    int currentPlayerID;
    std::string accountsFile;

    // Helper functions
    bool usernameExists(const std::string& username);
    bool isValidPassword(const std::string& password);
    int findPlayerIndex(const std::string& username);
    void loadAccounts();
    void saveAccounts();
    std::string getCurrentDate();

public:
    Authentication();
    ~Authentication();
    
    // Main functions
    bool registerPlayer(const std::string& username, const std::string& password, const std::string& email = "");
    bool login(const std::string& username, const std::string& password);
    int getCurrentPlayerID() const;
    std::string getCurrentUsername(int playerID);
    
    // UI functions
    bool showLoginScreen(sf::RenderWindow* window);
    bool showRegistrationScreen(sf::RenderWindow* window);
};
