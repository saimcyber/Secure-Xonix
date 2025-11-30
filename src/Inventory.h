#pragma once
#include <string>
#include <SFML/Graphics.hpp>
struct Theme {
    int themeID;
    std::string name;
    std::string description;
    sf::Color primaryColor;
    sf::Color secondaryColor;
    std::string backgroundImage;  // Path to background image
    int unlockLevel;  // Level required to unlock
    bool isPremium;   // Requires premium purchase
    Theme() : themeID(0), name(""), description(""), 
              primaryColor(sf::Color::White), secondaryColor(sf::Color::Black),
              backgroundImage(""), unlockLevel(0), isPremium(false) {}
    Theme(int id, std::string n, std::string desc, sf::Color pc, sf::Color sc, std::string bgImg, int level, bool premium)
        : themeID(id), name(n), description(desc), primaryColor(pc), secondaryColor(sc),
          backgroundImage(bgImg), unlockLevel(level), isPremium(premium) {}
};
struct ThemeNode {
    Theme theme;
    ThemeNode* left;
    ThemeNode* right;
    int height;  // Height of node for AVL balancing
    ThemeNode(Theme t) : theme(t), left(nullptr), right(nullptr), height(1) {}
};
class ThemeInventory {
private:
    ThemeNode* root;
    int getHeight(ThemeNode* node);
    int getBalanceFactor(ThemeNode* node);
    ThemeNode* rotateLeft(ThemeNode* node);
    ThemeNode* rotateRight(ThemeNode* node);
    ThemeNode* balanceNode(ThemeNode* node);
    ThemeNode* insertHelper(ThemeNode* node, Theme theme);
    ThemeNode* searchHelper(ThemeNode* node, int themeID);
    void inOrderHelper(ThemeNode* node);
    void destroyTree(ThemeNode* node);
    void displayThemeNode(ThemeNode* node, int& count);
public:
    ThemeInventory();
    ~ThemeInventory();
    void insertTheme(Theme theme);
    Theme* searchTheme(int themeID);
    void displayAllThemes();
    void displayInOrder();
    void initializeDefaultThemes();
};
struct PlayerInventory {
    int playerID;
    bool unlockedThemes[20];  // Array for 20 themes
    int equippedThemeID;
    PlayerInventory() : playerID(0), equippedThemeID(1) {
        for (int i = 0; i < 20; i++) {
            unlockedThemes[i] = false;
        }
        unlockedThemes[0] = true; // Default theme always unlocked
    }
};
class InventoryManager {
private:
    ThemeInventory themeTree;
    PlayerInventory playerInventories[100];
    int playerCount;
    int findPlayerIndex(int playerID);
    void loadInventoryData();
    void saveInventoryData();
public:
    InventoryManager();
    ~InventoryManager();
    void addPlayer(int playerID);
    bool unlockTheme(int playerID, int themeID);
    bool equipTheme(int playerID, int themeID);
    void autoUnlockByLevel(int playerID, int playerLevel);
    Theme* getEquippedTheme(int playerID);
    void displayPlayerInventory(int playerID);
    void displayAvailableThemes(int playerID, int playerLevel);
    ThemeInventory* getThemeTree() { return &themeTree; }
    PlayerInventory* getPlayerInventory(int playerID);
};
