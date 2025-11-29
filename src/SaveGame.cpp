/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Save game system implementation
 */

#include "SaveGame.h"
#include "main.h"
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;
using namespace sf;
#include <sys/stat.h>  // For mkdir
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

extern const int M;
extern const int N;

// Constructor
SaveGame::SaveGame() {
    savesDirectory = "data/saves/";
    nextSaveID = 1;
    
    // Create saves directory if it doesn't exist
    MKDIR("data");
    MKDIR(savesDirectory.c_str());
    
    loadSaveIDCounter();
}

// Destructor
SaveGame::~SaveGame() {
    saveSaveIDCounter();
    
    // Clean up tile linked list if exists
    TileNode* current = currentState.tilesHead;
    while (current != nullptr) {
        TileNode* temp = current;
        current = current->next;
        delete temp;
    }
}

// Get current timestamp
string SaveGame::getCurrentTimestamp() {
    time_t now = time(0);
    char buffer[80];
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
    return string(buffer);
}

// Load save ID counter
void SaveGame::loadSaveIDCounter() {
    ifstream file(savesDirectory + "save_counter.txt");
    if (file.is_open()) {
        file >> nextSaveID;
        file.close();
    }
}

// Save save ID counter
void SaveGame::saveSaveIDCounter() {
    ofstream file(savesDirectory + "save_counter.txt");
    if (file.is_open()) {
        file << nextSaveID;
        file.close();
    }
}

// Save current game
bool SaveGame::saveGame(int playerID, const string& playerName, int gameMode, int difficulty,
                       int grid[25][40], int playerX, int playerY, int playerDX, int playerDY,
                       int score, int moveCount, int powerUpCount,
                       int p2X, int p2Y, int p2DX, int p2DY, int p2Score, int p2PowerUpCount,
                       void* enemies, int enemyCount, float gameTime) {
    
    currentState.saveID = nextSaveID++;
    currentState.timestamp = getCurrentTimestamp();
    currentState.playerID = playerID;
    currentState.playerName = playerName;
    currentState.gameMode = gameMode;
    currentState.difficulty = difficulty;
    
    // Player data
    currentState.playerX = playerX;
    currentState.playerY = playerY;
    currentState.playerDX = playerDX;
    currentState.playerDY = playerDY;
    currentState.score = score;
    currentState.moveCount = moveCount;
    currentState.powerUpCount = powerUpCount;
    
    // Multiplayer data
    currentState.player2X = p2X;
    currentState.player2Y = p2Y;
    currentState.player2DX = p2DX;
    currentState.player2DY = p2DY;
    currentState.player2Score = p2Score;
    currentState.player2PowerUpCount = p2PowerUpCount;
    
    // Enemy data
    currentState.enemyCount = enemyCount;
    if (enemies != nullptr) {
        Enemy* enemyArray = static_cast<Enemy*>(enemies);
        for (int i = 0; i < enemyCount && i < 100; i++) {
            currentState.enemies[i].x = enemyArray[i].x;
            currentState.enemies[i].y = enemyArray[i].y;
            currentState.enemies[i].dx = enemyArray[i].dx;
            currentState.enemies[i].dy = enemyArray[i].dy;
            currentState.enemies[i].speed = enemyArray[i].speed;
            currentState.enemies[i].moveType = enemyArray[i].moveType;
            currentState.enemies[i].patternTimer = enemyArray[i].patternTimer;
        }
    }
    
    // Game time
    currentState.totalGameTime = gameTime;
    
    // Build linked list of non-empty tiles
    // Clear existing list first
    TileNode* current = currentState.tilesHead;
    while (current != nullptr) {
        TileNode* temp = current;
        current = current->next;
        delete temp;
    }
    currentState.tilesHead = nullptr;
    
    // Add tiles to linked list (only non-zero tiles)
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++) {
            if (grid[y][x] != 0) {
                TileNode* newNode = new TileNode(x, y, grid[y][x]);
                newNode->next = currentState.tilesHead;
                currentState.tilesHead = newNode;
            }
        }
    }
    
    // Save to file
    string filename = savesDirectory + "save_" + to_string(currentState.saveID) + ".txt";
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not create save file!" << endl;
        return false;
    }
    
    // Write header
    file << currentState.saveID << "\n";
    file << currentState.timestamp << "\n";
    file << currentState.playerID << " " << currentState.playerName << "\n";
    file << currentState.gameMode << " " << currentState.difficulty << "\n";
    
    // Write player data
    file << currentState.playerX << " " << currentState.playerY << " "
         << currentState.playerDX << " " << currentState.playerDY << "\n";
    file << currentState.score << " " << currentState.moveCount << " " << currentState.powerUpCount << "\n";
    
    // Write multiplayer data
    file << currentState.player2X << " " << currentState.player2Y << " "
         << currentState.player2DX << " " << currentState.player2DY << "\n";
    file << currentState.player2Score << " " << currentState.player2PowerUpCount << "\n";
    
    // Write game time
    file << currentState.totalGameTime << "\n";
    
    // Write enemy data
    file << currentState.enemyCount << "\n";
    for (int i = 0; i < currentState.enemyCount; i++) {
        file << currentState.enemies[i].x << " " << currentState.enemies[i].y << " "
             << currentState.enemies[i].dx << " " << currentState.enemies[i].dy << " "
             << currentState.enemies[i].speed << " " << currentState.enemies[i].moveType << " "
             << currentState.enemies[i].patternTimer << "\n";
    }
    
    // Write tile count
    int tileCount = 0;
    current = currentState.tilesHead;
    while (current != nullptr) {
        tileCount++;
        current = current->next;
    }
    file << tileCount << "\n";
    
    // Write tiles
    current = currentState.tilesHead;
    while (current != nullptr) {
        file << current->x << " " << current->y << " " << current->state << "\n";
        current = current->next;
    }
    
    file.close();
    saveSaveIDCounter();
    
    cout << "Game saved successfully! Save ID: " << currentState.saveID << endl;
    return true;
}

// Load game
bool SaveGame::loadGame(int saveID, int grid[25][40], int& playerX, int& playerY, int& playerDX, int& playerDY,
                       int& score, int& moveCount, int& powerUpCount,
                       int& p2X, int& p2Y, int& p2DX, int& p2DY, int& p2Score, int& p2PowerUpCount,
                       void* enemies, int& enemyCount, float& gameTime,
                       int& gameMode, int& difficulty) {
    
    string filename = savesDirectory + "save_" + to_string(saveID) + ".txt";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Save file not found!" << endl;
        return false;
    }
    
    // Read header
    string timestamp, playerName;
    int pid;
    file >> currentState.saveID;
    file.ignore();
    getline(file, timestamp);
    file >> pid;
    file.ignore();
    getline(file, playerName);
    file >> gameMode >> difficulty;
    
    // Read player data
    file >> playerX >> playerY >> playerDX >> playerDY;
    file >> score >> moveCount >> powerUpCount;
    
    // Read multiplayer data
    file >> p2X >> p2Y >> p2DX >> p2DY;
    file >> p2Score >> p2PowerUpCount;
    
    // Read game time
    file >> gameTime;
    
    // Read enemy data
    file >> enemyCount;
    if (enemies != nullptr) {
        Enemy* enemyArray = static_cast<Enemy*>(enemies);
        for (int i = 0; i < enemyCount && i < 100; i++) {
            file >> enemyArray[i].x >> enemyArray[i].y
                 >> enemyArray[i].dx >> enemyArray[i].dy
                 >> enemyArray[i].speed >> enemyArray[i].moveType
                 >> enemyArray[i].patternTimer;
        }
    } else {
        // Skip enemy data
        for (int i = 0; i < enemyCount; i++) {
            int dummy;
            float fdummy;
            file >> dummy >> dummy >> dummy >> dummy >> fdummy >> dummy >> fdummy;
        }
    }
    
    // Clear grid
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++) {
            grid[y][x] = 0;
        }
    }
    
    // Read tiles
    int tileCount;
    file >> tileCount;
    for (int i = 0; i < tileCount; i++) {
        int x, y, state;
        file >> x >> y >> state;
        if (y >= 0 && y < M && x >= 0 && x < N) {
            grid[y][x] = state;
        }
    }
    
    file.close();
    
    cout << "Game loaded successfully! Save ID: " << saveID << endl;
    return true;
}

// Get latest save ID for player
int SaveGame::getLatestSaveID(int playerID) {
    int latestID = -1;
    
    for (int id = 1; id < nextSaveID; id++) {
        string filename = savesDirectory + "save_" + to_string(id) + ".txt";
        ifstream file(filename);
        if (file.is_open()) {
            int saveID, pid;
            string timestamp, playerName;
            
            file >> saveID;
            file.ignore();
            getline(file, timestamp);
            file >> pid;
            
            if (pid == playerID) {
                latestID = saveID;
            }
            file.close();
        }
    }
    
    return latestID;
}

// List saves for player
void SaveGame::listSaves(int playerID) {
    cout << "Saved games for player ID " << playerID << ":" << endl;
    
    for (int id = 1; id < nextSaveID; id++) {
        string filename = savesDirectory + "save_" + to_string(id) + ".txt";
        ifstream file(filename);
        if (file.is_open()) {
            int saveID, pid;
            string timestamp, playerName;
            
            file >> saveID;
            file.ignore();
            getline(file, timestamp);
            file >> pid;
            
            if (pid == playerID) {
                cout << "Save ID: " << saveID << " - " << timestamp << endl;
            }
            file.close();
        }
    }
}

// Delete save
bool SaveGame::deleteSave(int saveID) {
    string filename = savesDirectory + "save_" + to_string(saveID) + ".txt";
    if (remove(filename.c_str()) == 0) {
        cout << "Save deleted successfully!" << endl;
        return true;
    }
    return false;
}
