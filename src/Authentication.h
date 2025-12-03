/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
#include <SFML/Graphics.hpp>
using namespace std;
const int MAX_PLAYERS = 100;
struct PlayerAccount {
    string username;
    string password;
    string email;
    int playerID;
    string registrationDate;
    bool isActive;
};
class Authentication {
private:
    PlayerAccount players[MAX_PLAYERS];
    int playerCount;
    int currentPlayerID;
    string accountsFile;
    bool usernameExists(const string& username);
    bool isValidPassword(const string& password);
    int findPlayerIndex(const string& username);
    void loadAccounts();
    void saveAccounts();
    string getCurrentDate();
public:
    Authentication();
    ~Authentication();
    bool registerPlayer(const string& username, const string& password, const string& email = "");
    bool login(const string& username, const string& password);
    int getCurrentPlayerID() const;
    string getCurrentUsername(int playerID);
    bool showLoginScreen(sf::RenderWindow* window);
    bool showRegistrationScreen(sf::RenderWindow* window);
};
