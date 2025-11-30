/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
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
SaveGame::SaveGame() {
    savesDirectory = "data/saves/";
    nextSaveID = 1;
    MKDIR("data");
    MKDIR(savesDirectory.c_str());
    loadSaveIDCounter();
}
SaveGame::~SaveGame() {
    saveSaveIDCounter();
    TileNode* current = currentState.tilesHead;
    while (current != nullptr) {
        TileNode* temp = current;
        current = current->next;
        delete temp;
    }
}
string SaveGame::getCurrentTimestamp() {
    time_t now = time(0);
    char buffer[80];
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
    return string(buffer);
}
void SaveGame::loadSaveIDCounter() {
    ifstream file(savesDirectory + "save_counter.txt");
    if (file.is_open()) {
        file >> nextSaveID;
        file.close();
    }
}
void SaveGame::saveSaveIDCounter() {
    ofstream file(savesDirectory + "save_counter.txt");
    if (file.is_open()) {
        file << nextSaveID;
        file.close();
    }
}
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
    currentState.playerX = playerX;
    currentState.playerY = playerY;
    currentState.playerDX = playerDX;
    currentState.playerDY = playerDY;
    currentState.score = score;
    currentState.moveCount = moveCount;
    currentState.powerUpCount = powerUpCount;
    currentState.player2X = p2X;
    currentState.player2Y = p2Y;
    currentState.player2DX = p2DX;
    currentState.player2DY = p2DY;
    currentState.player2Score = p2Score;
    currentState.player2PowerUpCount = p2PowerUpCount;
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
    currentState.totalGameTime = gameTime;
    TileNode* current = currentState.tilesHead;
    while (current != nullptr) {
        TileNode* temp = current;
        current = current->next;
        delete temp;
    }
    currentState.tilesHead = nullptr;
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++) {
            if (grid[y][x] != 0) {
                TileNode* newNode = new TileNode(x, y, grid[y][x]);
                newNode->next = currentState.tilesHead;
                currentState.tilesHead = newNode;
            }
        }
    }
    string filename = savesDirectory + "save_" + to_string(currentState.saveID) + ".txt";
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not create save file!" << endl;
        return false;
    }
    file << currentState.saveID << "\n";
    file << currentState.timestamp << "\n";
    file << currentState.playerID << " " << currentState.playerName << "\n";
    file << currentState.gameMode << " " << currentState.difficulty << "\n";
    file << currentState.playerX << " " << currentState.playerY << " "
         << currentState.playerDX << " " << currentState.playerDY << "\n";
    file << currentState.score << " " << currentState.moveCount << " " << currentState.powerUpCount << "\n";
    file << currentState.player2X << " " << currentState.player2Y << " "
         << currentState.player2DX << " " << currentState.player2DY << "\n";
    file << currentState.player2Score << " " << currentState.player2PowerUpCount << "\n";
    file << currentState.totalGameTime << "\n";
    file << currentState.enemyCount << "\n";
    for (int i = 0; i < currentState.enemyCount; i++) {
        file << currentState.enemies[i].x << " " << currentState.enemies[i].y << " "
             << currentState.enemies[i].dx << " " << currentState.enemies[i].dy << " "
             << currentState.enemies[i].speed << " " << currentState.enemies[i].moveType << " "
             << currentState.enemies[i].patternTimer << "\n";
    }
    int tileCount = 0;
    current = currentState.tilesHead;
    while (current != nullptr) {
        tileCount++;
        current = current->next;
    }
    file << tileCount << "\n";
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
    string timestamp, playerName;
    int pid;
    file >> currentState.saveID;
    file.ignore();
    getline(file, timestamp);
    file >> pid;
    file.ignore();
    getline(file, playerName);
    file >> gameMode >> difficulty;
    file >> playerX >> playerY >> playerDX >> playerDY;
    file >> score >> moveCount >> powerUpCount;
    file >> p2X >> p2Y >> p2DX >> p2DY;
    file >> p2Score >> p2PowerUpCount;
    file >> gameTime;
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
        for (int i = 0; i < enemyCount; i++) {
            int dummy;
            float fdummy;
            file >> dummy >> dummy >> dummy >> dummy >> fdummy >> dummy >> fdummy;
        }
    }
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++) {
            grid[y][x] = 0;
        }
    }
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
bool SaveGame::deleteSave(int saveID) {
    string filename = savesDirectory + "save_" + to_string(saveID) + ".txt";
    if (remove(filename.c_str()) == 0) {
        cout << "Save deleted successfully!" << endl;
        return true;
    }
    return false;
}
