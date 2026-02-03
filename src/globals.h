/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
using namespace std;

// Forward declarations
class FriendSystem;
class InventoryManager;
class SaveGame;

// Grid dimensions
const int M = 25;
const int N = 40;

// Globals namespace with getter/setter functions
namespace Globals {
    // Getter and setter functions
    int& getCurrentPlayerID();
    string& getCurrentUsername();
    FriendSystem*& getFriendSystem();
    InventoryManager*& getInventoryManager();
    SaveGame*& getSaveGame();
    int& getDifficulty();
}

// Convenience macros for backward compatibility (optional)
#define g_currentPlayerID Globals::getCurrentPlayerID()
#define g_currentUsername Globals::getCurrentUsername()
#define g_friendSystem Globals::getFriendSystem()
#define g_inventoryMgr Globals::getInventoryManager()
#define g_saveGame Globals::getSaveGame()
#define difficulty Globals::getDifficulty()
