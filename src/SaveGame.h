/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Save game system using linked list for tile state
 */

#pragma once

#include <string>
#include <SFML/Graphics.hpp>

// Tile node for linked list
struct TileNode {
    int x, y;
    int state;  // 0=empty, 1=wall, 2=trail, 3=p1trail, 4=p2trail
    TileNode* next;
    
    TileNode(int _x, int _y, int _state) : x(_x), y(_y), state(_state), next(nullptr) {}
};

// Enemy save data
struct SavedEnemy {
    int x, y, dx, dy;
    float speed;
    int moveType;
    float patternTimer;
};

// Game state structure
struct GameState {
    int saveID;
    std::string timestamp;
    int playerID;
    std::string playerName;
    
    // Game mode and difficulty
    int gameMode;  // 1=single, 2=multi
    int difficulty;
    
    // Player data
    int playerX, playerY, playerDX, playerDY;
    int score;
    int moveCount;
    int powerUpCount;
    
    // Multiplayer data
    int player2X, player2Y, player2DX, player2DY;
    int player2Score;
    int player2PowerUpCount;
    
    // Enemies
    int enemyCount;
    SavedEnemy enemies[100];
    
    // Game timing
    float totalGameTime;
    
    // Tile state (linked list)
    TileNode* tilesHead;
    
    GameState() : tilesHead(nullptr) {}
};

// Save game class
class SaveGame {
private:
    GameState currentState;
    std::string savesDirectory;
    int nextSaveID;
    
    void loadSaveIDCounter();
    void saveSaveIDCounter();
    std::string getCurrentTimestamp();
    
public:
    SaveGame();
    ~SaveGame();
    
    // Save current game state
    bool saveGame(int playerID, const std::string& playerName, int gameMode, int difficulty,
                  int grid[25][40], int playerX, int playerY, int playerDX, int playerDY,
                  int score, int moveCount, int powerUpCount,
                  int p2X, int p2Y, int p2DX, int p2DY, int p2Score, int p2PowerUpCount,
                  void* enemies, int enemyCount, float gameTime);
    
    // Load game state
    bool loadGame(int saveID, int grid[25][40], int& playerX, int& playerY, int& playerDX, int& playerDY,
                  int& score, int& moveCount, int& powerUpCount,
                  int& p2X, int& p2Y, int& p2DX, int& p2DY, int& p2Score, int& p2PowerUpCount,
                  void* enemies, int& enemyCount, float& gameTime,
                  int& gameMode, int& difficulty);
    
    // List available saves for a player
    void listSaves(int playerID);
    int getLatestSaveID(int playerID);
    
    // Delete save
    bool deleteSave(int saveID);
};
