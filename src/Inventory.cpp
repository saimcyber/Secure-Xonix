/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#include "Inventory.h"
#include <iostream>
#include <fstream>
using namespace std;
ThemeInventory::ThemeInventory() {
    root = nullptr;
    initializeDefaultThemes();
}
ThemeInventory::~ThemeInventory() {
    destroyTree(root);
}
void ThemeInventory::destroyTree(ThemeNode* node) {
    if (node) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}
int ThemeInventory::getHeight(ThemeNode* node) {
    if (node == nullptr) return 0;
    return node->height;
}
int ThemeInventory::getBalanceFactor(ThemeNode* node) {
    if (node == nullptr) return 0;
    return getHeight(node->left) - getHeight(node->right);
}
ThemeNode* ThemeInventory::rotateLeft(ThemeNode* x) {
    ThemeNode* y = x->right;
    ThemeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + max(getHeight(x->left), getHeight(x->right));
    y->height = 1 + max(getHeight(y->left), getHeight(y->right));
    return y;  // New root
}
ThemeNode* ThemeInventory::rotateRight(ThemeNode* y) {
    ThemeNode* x = y->left;
    ThemeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + max(getHeight(y->left), getHeight(y->right));
    x->height = 1 + max(getHeight(x->left), getHeight(x->right));
    return x;  // New root
}
ThemeNode* ThemeInventory::balanceNode(ThemeNode* node) {
    if (node == nullptr) return node;
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalanceFactor(node);
    if (balance > 1 && getBalanceFactor(node->left) >= 0) {
        return rotateRight(node);
    }
    if (balance > 1 && getBalanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && getBalanceFactor(node->right) <= 0) {
        return rotateLeft(node);
    }
    if (balance < -1 && getBalanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;  // Already balanced
}
ThemeNode* ThemeInventory::insertHelper(ThemeNode* node, Theme theme) {
    if (node == nullptr) {
        return new ThemeNode(theme);
    }
    if (theme.themeID < node->theme.themeID) {
        node->left = insertHelper(node->left, theme);
    } else if (theme.themeID > node->theme.themeID) {
        node->right = insertHelper(node->right, theme);
    } else {
        return node;  // Duplicate, don't insert
    }
    return balanceNode(node);
}
void ThemeInventory::insertTheme(Theme theme) {
    root = insertHelper(root, theme);
}
ThemeNode* ThemeInventory::searchHelper(ThemeNode* node, int themeID) {
    if (node == nullptr || node->theme.themeID == themeID) {
        return node;
    }
    if (themeID < node->theme.themeID) {
        return searchHelper(node->left, themeID);
    } else {
        return searchHelper(node->right, themeID);
    }
}
Theme* ThemeInventory::searchTheme(int themeID) {
    ThemeNode* node = searchHelper(root, themeID);
    if (node) {
        return &(node->theme);
    }
    return nullptr;
}
void ThemeInventory::inOrderHelper(ThemeNode* node) {
    if (node) {
        inOrderHelper(node->left);
        cout << "ID: " << node->theme.themeID << " - " << node->theme.name << endl;
        inOrderHelper(node->right);
    }
}
void ThemeInventory::displayInOrder() {
    cout << "\n=== All Themes (Sorted by ID) ===" << endl;
    inOrderHelper(root);
}
void ThemeInventory::displayThemeNode(ThemeNode* node, int& count) {
    if (node) {
        displayThemeNode(node->left, count);
        count++;
        cout << count << ". " << node->theme.name << endl;
        cout << "   Description: " << node->theme.description << endl;
        cout << "   Unlock Level: " << node->theme.unlockLevel << endl;
        if (node->theme.isPremium) {
            cout << "   [PREMIUM]" << endl;
        }
        cout << endl;
        displayThemeNode(node->right, count);
    }
}
void ThemeInventory::displayAllThemes() {
    int count = 0;
    displayThemeNode(root, count);
}
void ThemeInventory::initializeDefaultThemes() {
    insertTheme(Theme(1, "Classic", "Original Xonix theme", 
                     sf::Color::Blue, sf::Color::Cyan, "", 0, false));
    insertTheme(Theme(2, "Ocean", "Deep blue waters", 
                     sf::Color(0, 105, 148), sf::Color(173, 216, 230), "assets/images/ocean_bg.jpg", 5, false));
    insertTheme(Theme(3, "Forest", "Green nature theme", 
                     sf::Color(34, 139, 34), sf::Color(144, 238, 144), "assets/images/forest_bg.jpg", 10, false));
    insertTheme(Theme(4, "Sunset", "Warm orange and pink", 
                     sf::Color(255, 140, 0), sf::Color(255, 182, 193), "assets/images/sunset_bg.jpg", 15, false));
    insertTheme(Theme(5, "Night", "Dark purple sky", 
                     sf::Color(25, 25, 112), sf::Color(138, 43, 226), "assets/images/night_bg.jpg", 20, false));
    insertTheme(Theme(6, "Fire", "Red hot flames", 
                     sf::Color(220, 20, 60), sf::Color(255, 69, 0), "assets/images/fire_bg.jpg", 25, true));
    insertTheme(Theme(7, "Ice", "Cool cyan winter", 
                     sf::Color(0, 191, 255), sf::Color(224, 255, 255), "assets/images/ice_bg.jpeg", 30, true));
    insertTheme(Theme(8, "Desert", "Sandy golden dunes", 
                     sf::Color(210, 180, 140), sf::Color(255, 215, 0), "assets/images/desert_bg.jpg", 35, true));
    insertTheme(Theme(9, "Space", "Cosmic black and stars", 
                     sf::Color(0, 0, 0), sf::Color(255, 255, 255), "assets/images/space_bg.jpg", 40, true));
    insertTheme(Theme(10, "Rainbow", "Bright colorful spectrum", 
                     sf::Color(255, 0, 255), sf::Color(255, 255, 0), "assets/images/rainbow_bg.jpg", 50, true));
}
InventoryManager::InventoryManager() {
    playerCount = 0;
    loadInventoryData();
}
InventoryManager::~InventoryManager() {
    saveInventoryData();
}
int InventoryManager::findPlayerIndex(int playerID) {
    for (int i = 0; i < playerCount; i++) {
        if (playerInventories[i].playerID == playerID) {
            return i;
        }
    }
    return -1;
}
void InventoryManager::addPlayer(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index != -1) return; // Already exists
    if (playerCount >= 100) {
        cout << "Maximum players reached!" << endl;
        return;
    }
    playerInventories[playerCount].playerID = playerID;
    playerInventories[playerCount].equippedThemeID = 1; // Default theme
    for (int i = 0; i < 20; i++) {
        playerInventories[playerCount].unlockedThemes[i] = false;
    }
    playerInventories[playerCount].unlockedThemes[0] = true; // Theme 1 unlocked
    playerCount++;
}
bool InventoryManager::unlockTheme(int playerID, int themeID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) {
        addPlayer(playerID);
        index = findPlayerIndex(playerID);
    }
    if (themeID < 1 || themeID > 20) {
        cout << "Invalid theme ID!" << endl;
        return false;
    }
    if (playerInventories[index].unlockedThemes[themeID - 1]) {
        cout << "Theme already unlocked!" << endl;
        return false;
    }
    playerInventories[index].unlockedThemes[themeID - 1] = true;
    saveInventoryData();  // Save immediately after unlock
    cout << "Theme unlocked successfully!" << endl;
    return true;
}
bool InventoryManager::equipTheme(int playerID, int themeID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) {
        cout << "Player not found!" << endl;
        return false;
    }
    if (themeID < 1 || themeID > 20) {
        cout << "Invalid theme ID!" << endl;
        return false;
    }
    if (!playerInventories[index].unlockedThemes[themeID - 1]) {
        cout << "Theme not unlocked!" << endl;
        return false;
    }
    playerInventories[index].equippedThemeID = themeID;
    saveInventoryData();  // Save immediately after equip
    cout << "Theme equipped!" << endl;
    return true;
}
Theme* InventoryManager::getEquippedTheme(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return nullptr;
    return themeTree.searchTheme(playerInventories[index].equippedThemeID);
}
void InventoryManager::displayPlayerInventory(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) {
        cout << "Player not found!" << endl;
        return;
    }
    cout << "\n=== Your Inventory ===" << endl;
    cout << "Equipped Theme ID: " << playerInventories[index].equippedThemeID << endl;
    cout << "\nUnlocked Themes:" << endl;
    int count = 0;
    for (int i = 0; i < 20; i++) {
        if (playerInventories[index].unlockedThemes[i]) {
            Theme* theme = themeTree.searchTheme(i + 1);
            if (theme) {
                count++;
                cout << count << ". " << theme->name;
                if (i + 1 == playerInventories[index].equippedThemeID) {
                    cout << " [EQUIPPED]";
                }
                cout << endl;
            }
        }
    }
    if (count == 0) {
        cout << "No themes unlocked." << endl;
    }
}
void InventoryManager::displayAvailableThemes(int playerID, int playerLevel) {
    int index = findPlayerIndex(playerID);
    if (index == -1) {
        cout << "Player not found!" << endl;
        return;
    }
    cout << "\n=== Available Themes ===" << endl;
    cout << "Your Level: " << playerLevel << endl << endl;
    for (int i = 1; i <= 10; i++) {
        Theme* theme = themeTree.searchTheme(i);
        if (theme) {
            cout << i << ". " << theme->name;
            if (playerInventories[index].unlockedThemes[i - 1]) {
                cout << " [UNLOCKED]";
            } else if (theme->unlockLevel > playerLevel) {
                cout << " [LOCKED - Level " << theme->unlockLevel << " required]";
            } else if (theme->isPremium) {
                cout << " [PREMIUM - Purchase required]";
            } else {
                cout << " [Can unlock now!]";
            }
            cout << endl;
        }
    }
}
void InventoryManager::autoUnlockByLevel(int playerID, int playerLevel) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return;
    bool newUnlock = false;
    for (int i = 1; i <= 10; i++) {
        Theme* theme = themeTree.searchTheme(i);
        if (theme && !theme->isPremium && theme->unlockLevel <= playerLevel) {
            if (!playerInventories[index].unlockedThemes[i - 1]) {
                playerInventories[index].unlockedThemes[i - 1] = true;
                newUnlock = true;
                cout << "🎉 New theme unlocked: " << theme->name << "!" << endl;
            }
        }
    }
    if (newUnlock) {
        saveInventoryData();
    }
}
PlayerInventory* InventoryManager::getPlayerInventory(int playerID) {
    int index = findPlayerIndex(playerID);
    if (index == -1) return nullptr;
    return &playerInventories[index];
}
void InventoryManager::loadInventoryData() {
    ifstream file("data/inventory.txt");
    if (!file.is_open()) {
        cout << "No existing inventory data. Starting fresh." << endl;
        return;
    }
    int savedPlayerCount;
    file >> savedPlayerCount;
    playerCount = 0;
    for (int i = 0; i < savedPlayerCount && i < 100; i++) {
        int pID, equippedID;
        file >> pID >> equippedID;
        playerInventories[playerCount].playerID = pID;
        playerInventories[playerCount].equippedThemeID = equippedID;
        for (int j = 0; j < 20; j++) {
            int unlocked;
            file >> unlocked;
            playerInventories[playerCount].unlockedThemes[j] = (unlocked == 1);
        }
        playerCount++;
    }
    file.close();
    cout << "Loaded inventory data for " << playerCount << " players." << endl;
}
void InventoryManager::saveInventoryData() {
    ofstream file("data/inventory.txt");
    if (!file.is_open()) {
        cout << "Error: Could not save inventory data!" << endl;
        return;
    }
    file << playerCount << "\n";
    for (int i = 0; i < playerCount; i++) {
        file << playerInventories[i].playerID << " ";
        file << playerInventories[i].equippedThemeID << " ";
        for (int j = 0; j < 20; j++) {
            file << (playerInventories[i].unlockedThemes[j] ? 1 : 0) << " ";
        }
        file << "\n";
    }
    file.close();
    cout << "Inventory data saved successfully." << endl;
}
