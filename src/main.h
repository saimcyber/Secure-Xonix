/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
extern const int M;
extern const int N;
extern int g_currentPlayerID;
extern std::string g_currentUsername;
struct Enemy {
    int x, y, dx, dy;
    float speed;
    int moveType;
    float patternTimer;
    Enemy();
    void move(float deltaTime);
};
void updateScore(int capturedTiles);
void updateTop5Scores(int score, float totalGameTime);
void displayTopScores(sf::RenderWindow* window, sf::Font& font);
void drop(int y, int x);
void moveZigZag(Enemy& enemy, float deltaTime);
void moveDriftingSpiral(Enemy& enemy, float deltaTime);
void SingleGame(sf::RenderWindow* window, int difficulty, int playerID, const std::string& username);
void SingleGame(sf::RenderWindow* window, int difficulty, int playerID, const std::string& username, 
                int loadedGrid[25][40], int startX, int startY, int startDX, int startDY,
                int startScore, int startMoves, int startPowerUps, 
                Enemy* loadedEnemies, int loadedEnemyCount, float startTime);
void SingleGameWithSave(sf::RenderWindow* window, int saveID);
void MultiGame(sf::RenderWindow* window, int difficulty, int p1ID, const std::string& p1Name, int p2ID, const std::string& p2Name);
