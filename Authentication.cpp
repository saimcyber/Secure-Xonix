/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: [Student Name 1], [Student Name 2]
 * Roll Numbers: [Roll# 1], [Roll# 2]
 * Date: November 2025
 * Description: Authentication system implementation
 */

#include "Authentication.h"
#include "FriendSystem.h"
#include "Inventory.h"
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;
using namespace sf;

// Constructor
Authentication::Authentication() {
    playerCount = 0;
    currentPlayerID = -1;
    accountsFile = "accounts.txt";
    loadAccounts();
}

// Destructor
Authentication::~Authentication() {
    saveAccounts();
}

// Get current date as string
string Authentication::getCurrentDate() {
    time_t now = time(0);
    char buffer[80];
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    return string(buffer);
}

// Check if username already exists
bool Authentication::usernameExists(const string& username) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].username == username) {
            return true;
        }
    }
    return false;
}

// Validate password strength
bool Authentication::isValidPassword(const string& password) {
    // Minimum length check
    if (password.length() < 4) {
        return false;
    }
    return true;
}

// Find player index by username
int Authentication::findPlayerIndex(const string& username) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].username == username) {
            return i;
        }
    }
    return -1;
}

// Load accounts from file
void Authentication::loadAccounts() {
    ifstream file(accountsFile);
    if (!file.is_open()) {
        cout << "No existing accounts file found. Creating new one." << endl;
        return;
    }

    playerCount = 0;
    while (playerCount < MAX_PLAYERS && !file.eof()) {
        file >> players[playerCount].playerID;
        file >> players[playerCount].username;
        file >> players[playerCount].password;
        file >> players[playerCount].email;
        file.ignore(); // ignore newline
        getline(file, players[playerCount].registrationDate);
        
        if (file.fail()) break;
        
        players[playerCount].isActive = true;
        playerCount++;
    }
    file.close();
    cout << "Loaded " << playerCount << " player accounts." << endl;
    
    // Sync loaded players with Friend System and Inventory Manager
    extern FriendSystem g_friendSystem;
    extern InventoryManager g_inventoryMgr;
    for (int i = 0; i < playerCount; i++) {
        g_friendSystem.addPlayer(players[i].playerID, players[i].username);
        g_inventoryMgr.addPlayer(players[i].playerID);
    }
}

// Save accounts to file
void Authentication::saveAccounts() {
    ofstream file(accountsFile);
    if (!file.is_open()) {
        cout << "Error: Could not save accounts!" << endl;
        return;
    }

    for (int i = 0; i < playerCount; i++) {
        file << players[i].playerID << " "
             << players[i].username << " "
             << players[i].password << " "
             << players[i].email << " "
             << players[i].registrationDate << "\n";
    }
    file.close();
}

// Register new player
bool Authentication::registerPlayer(const string& username, const string& password, const string& email) {
    // Check if username already exists
    if (usernameExists(username)) {
        cout << "Username already taken!" << endl;
        return false;
    }

    // Validate password
    if (!isValidPassword(password)) {
        cout << "Password too weak! Minimum 4 characters required." << endl;
        return false;
    }

    // Check if we have space
    if (playerCount >= MAX_PLAYERS) {
        cout << "Maximum players reached!" << endl;
        return false;
    }

    // Create new account
    players[playerCount].playerID = playerCount + 1;
    players[playerCount].username = username;
    players[playerCount].password = password;
    players[playerCount].email = email.empty() ? "none" : email;
    players[playerCount].registrationDate = getCurrentDate();
    players[playerCount].isActive = true;

    int newPlayerID = playerCount + 1;
    playerCount++;
    saveAccounts();
    
    // Add player to Friend System and Inventory Manager
    extern FriendSystem g_friendSystem;
    extern InventoryManager g_inventoryMgr;
    g_friendSystem.addPlayer(newPlayerID, username);
    g_inventoryMgr.addPlayer(newPlayerID);
    
    cout << "Registration successful! Welcome, " << username << "!" << endl;
    return true;
}

// Login player
bool Authentication::login(const string& username, const string& password) {
    int index = findPlayerIndex(username);
    
    if (index == -1) {
        cout << "Username not found!" << endl;
        return false;
    }

    if (players[index].password != password) {
        cout << "Incorrect password!" << endl;
        return false;
    }

    currentPlayerID = players[index].playerID;
    cout << "Login successful! Welcome back, " << username << "!" << endl;
    return true;
}

// Get current player ID
int Authentication::getCurrentPlayerID() const {
    return currentPlayerID;
}

// Get username by player ID
string Authentication::getCurrentUsername(int playerID) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].playerID == playerID) {
            return players[i].username;
        }
    }
    return "Unknown";
}

// Show login screen
bool Authentication::showLoginScreen(RenderWindow* window) {
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return false;
    }

    // Load background
    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    // Title
    Text title("LOGIN", font, 40);
    title.setFillColor(Color::Yellow);
    title.setPosition(300, 50);

    // Input fields
    string username = "";
    string password = "";
    bool typingUsername = true;

    Text usernameLabel("Username:", font, 20);
    usernameLabel.setPosition(100, 150);
    usernameLabel.setFillColor(Color::White);

    Text usernameInput("", font, 20);
    usernameInput.setPosition(100, 180);
    usernameInput.setFillColor(Color::Green);

    Text passwordLabel("Password:", font, 20);
    passwordLabel.setPosition(100, 230);
    passwordLabel.setFillColor(Color::White);

    Text passwordInput("", font, 20);
    passwordInput.setPosition(100, 260);
    passwordInput.setFillColor(Color::Green);

    Text instruction("Press TAB to switch fields, ENTER to login, ESC to cancel", font, 16);
    instruction.setPosition(50, 350);
    instruction.setFillColor(Color(200, 200, 200));

    Text errorMsg("", font, 18);
    errorMsg.setPosition(100, 320);
    errorMsg.setFillColor(Color::Red);

    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape) {
                    return false;
                }
                if (event.key.code == Keyboard::Tab) {
                    typingUsername = !typingUsername;
                }
                if (event.key.code == Keyboard::Return) {
                    if (login(username, password)) {
                        return true;
                    } else {
                        errorMsg.setString("Login failed! Check credentials.");
                    }
                }
                if (event.key.code == Keyboard::BackSpace) {
                    if (typingUsername && username.length() > 0) {
                        username.pop_back();
                    } else if (!typingUsername && password.length() > 0) {
                        password.pop_back();
                    }
                }
            }

            if (event.type == Event::TextEntered) {
                if (event.text.unicode < 128 && event.text.unicode != 13 && 
                    event.text.unicode != 8 && event.text.unicode != 9) {
                    char ch = static_cast<char>(event.text.unicode);
                    if (typingUsername && username.length() < 20) {
                        username += ch;
                    } else if (!typingUsername && password.length() < 20) {
                        password += ch;
                    }
                }
            }
        }

        // Update display
        usernameInput.setString(username + (typingUsername ? "_" : ""));
        string displayPassword = "";
        for (size_t i = 0; i < password.length(); i++) displayPassword += "*";
        passwordInput.setString(displayPassword + (!typingUsername ? "_" : ""));

        // Render
        window->clear();
        window->draw(background);
        window->draw(title);
        window->draw(usernameLabel);
        window->draw(usernameInput);
        window->draw(passwordLabel);
        window->draw(passwordInput);
        window->draw(instruction);
        window->draw(errorMsg);
        window->display();
    }

    return false;
}

// Show registration screen
bool Authentication::showRegistrationScreen(RenderWindow* window) {
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        return false;
    }

    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    Text title("REGISTER NEW PLAYER", font, 35);
    title.setFillColor(Color::Yellow);
    title.setPosition(180, 50);

    string username = "";
    string password = "";
    string email = "";
    int activeField = 0; // 0=username, 1=password, 2=email

    Text usernameLabel("Username:", font, 20);
    usernameLabel.setPosition(100, 130);
    usernameLabel.setFillColor(Color::White);

    Text usernameInput("", font, 20);
    usernameInput.setPosition(100, 160);

    Text passwordLabel("Password (min 4 chars):", font, 20);
    passwordLabel.setPosition(100, 200);
    passwordLabel.setFillColor(Color::White);

    Text passwordInput("", font, 20);
    passwordInput.setPosition(100, 230);

    Text emailLabel("Email (optional):", font, 20);
    emailLabel.setPosition(100, 270);
    emailLabel.setFillColor(Color::White);

    Text emailInput("", font, 20);
    emailInput.setPosition(100, 300);

    Text instruction("TAB to switch, ENTER to register, ESC to cancel", font, 16);
    instruction.setPosition(100, 370);
    instruction.setFillColor(Color(200, 200, 200));

    Text errorMsg("", font, 18);
    errorMsg.setPosition(100, 340);
    errorMsg.setFillColor(Color::Red);

    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape) {
                    return false;
                }
                if (event.key.code == Keyboard::Tab) {
                    activeField = (activeField + 1) % 3;
                }
                if (event.key.code == Keyboard::Return) {
                    if (registerPlayer(username, password, email)) {
                        return true;
                    } else {
                        errorMsg.setString("Registration failed!");
                    }
                }
                if (event.key.code == Keyboard::BackSpace) {
                    if (activeField == 0 && username.length() > 0) username.pop_back();
                    else if (activeField == 1 && password.length() > 0) password.pop_back();
                    else if (activeField == 2 && email.length() > 0) email.pop_back();
                }
            }

            if (event.type == Event::TextEntered) {
                if (event.text.unicode < 128 && event.text.unicode != 13 && 
                    event.text.unicode != 8 && event.text.unicode != 9) {
                    char ch = static_cast<char>(event.text.unicode);
                    if (activeField == 0 && username.length() < 20) username += ch;
                    else if (activeField == 1 && password.length() < 20) password += ch;
                    else if (activeField == 2 && email.length() < 30) email += ch;
                }
            }
        }

        // Update colors based on active field
        usernameInput.setFillColor(activeField == 0 ? Color::Green : Color::White);
        passwordInput.setFillColor(activeField == 1 ? Color::Green : Color::White);
        emailInput.setFillColor(activeField == 2 ? Color::Green : Color::White);

        // Update display
        usernameInput.setString(username + (activeField == 0 ? "_" : ""));
        
        string displayPassword = "";
        for (size_t i = 0; i < password.length(); i++) displayPassword += "*";
        passwordInput.setString(displayPassword + (activeField == 1 ? "_" : ""));
        
        emailInput.setString(email + (activeField == 2 ? "_" : ""));

        // Render
        window->clear();
        window->draw(background);
        window->draw(title);
        window->draw(usernameLabel);
        window->draw(usernameInput);
        window->draw(passwordLabel);
        window->draw(passwordInput);
        window->draw(emailLabel);
        window->draw(emailInput);
        window->draw(instruction);
        window->draw(errorMsg);
        window->display();
    }

    return false;
}
