#pragma once
#include <string>
#include <SFML/Graphics.hpp>
const int MAX_PLAYERS = 100;
struct PlayerAccount {
    std::string username;
    std::string password;
    std::string email;
    int playerID;
    std::string registrationDate;
    bool isActive;
};
class Authentication {
private:
    PlayerAccount players[MAX_PLAYERS];
    int playerCount;
    int currentPlayerID;
    std::string accountsFile;
    bool usernameExists(const std::string& username);
    bool isValidPassword(const std::string& password);
    int findPlayerIndex(const std::string& username);
    void loadAccounts();
    void saveAccounts();
    std::string getCurrentDate();
public:
    Authentication();
    ~Authentication();
    bool registerPlayer(const std::string& username, const std::string& password, const std::string& email = "");
    bool login(const std::string& username, const std::string& password);
    int getCurrentPlayerID() const;
    std::string getCurrentUsername(int playerID);
    bool showLoginScreen(sf::RenderWindow* window);
    bool showRegistrationScreen(sf::RenderWindow* window);
};
