/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: [Student Name 1], [Student Name 2]
 * Roll Numbers: [Roll# 1], [Roll# 2]
 * Date: November 2025
 * Description: Header file declaring game logic functions and structures
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

// Grid dimensions
extern const int M;
extern const int N;

// Global player info (set after authentication)
extern int g_currentPlayerID;
extern std::string g_currentUsername;

// Enemy structure definition (needed by SaveGame)
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
void MultiGame(sf::RenderWindow* window, int difficulty, int p1ID, const std::string& p1Name, int p2ID, const std::string& p2Name);
