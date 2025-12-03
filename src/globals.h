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

// Global variable definitions (static to avoid multiple definition errors)
static int g_currentPlayerID = -1;
static string g_currentUsername = "";
static FriendSystem* g_friendSystem = nullptr;
static InventoryManager* g_inventoryMgr = nullptr;
static SaveGame* g_saveGame = nullptr;
static int difficulty = 1;
