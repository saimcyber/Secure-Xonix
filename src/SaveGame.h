/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
#include <SFML/Graphics.hpp>
struct TileNode {
    int x, y;
    int state;  // 0=empty, 1=wall, 2=trail, 3=p1trail, 4=p2trail
    TileNode* next;
    TileNode(int _x, int _y, int _state) : x(_x), y(_y), state(_state), next(nullptr) {}
};
struct SavedEnemy {
    int x, y, dx, dy;
    float speed;
    int moveType;
    float patternTimer;
};
struct GameState {
    int saveID;
    std::string timestamp;
    int playerID;
    std::string playerName;
    int gameMode;  // 1=single, 2=multi
    int difficulty;
    int playerX, playerY, playerDX, playerDY;
    int score;
    int moveCount;
    int powerUpCount;
    int player2X, player2Y, player2DX, player2DY;
    int player2Score;
    int player2PowerUpCount;
    int enemyCount;
    SavedEnemy enemies[100];
    float totalGameTime;
    TileNode* tilesHead;
    GameState() : tilesHead(nullptr) {}
};
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
    bool saveGame(int playerID, const std::string& playerName, int gameMode, int difficulty,
                  int grid[25][40], int playerX, int playerY, int playerDX, int playerDY,
                  int score, int moveCount, int powerUpCount,
                  int p2X, int p2Y, int p2DX, int p2DY, int p2Score, int p2PowerUpCount,
                  void* enemies, int enemyCount, float gameTime);
    bool loadGame(int saveID, int grid[25][40], int& playerX, int& playerY, int& playerDX, int& playerDY,
                  int& score, int& moveCount, int& powerUpCount,
                  int& p2X, int& p2Y, int& p2DX, int& p2DY, int& p2Score, int& p2PowerUpCount,
                  void* enemies, int& enemyCount, float& gameTime,
                  int& gameMode, int& difficulty);
    void listSaves(int playerID);
    int getLatestSaveID(int playerID);
    bool deleteSave(int saveID);
};
