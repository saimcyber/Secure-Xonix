#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip> 
#include <cstring>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Menu.h"
#include "main.h"
#include "Authentication.h"
#include "Profile.h"
#include "MinHeap.h"
#include "FriendSystem.h"
#include "Inventory.h"
#include "SaveGame.h"
#include <time.h>
using namespace sf;
using namespace std;
template<typename T>
T clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}
const int M = 25;
const int N = 40;
int grid[M][N] = {0};
int ts = 18; // tile size
int score = 0; // Score counter
bool isPaused = false;
float totalGameTime = 0.0f; // stores total time when paused
int moveCount = 0; // Movement counter
int g_currentPlayerID = -1;
string g_currentUsername = "";
FriendSystem* g_friendSystem = nullptr;
InventoryManager* g_inventoryMgr = nullptr;
SaveGame* g_saveGame = nullptr;
void updateScore(int capturedTiles) {
    score += capturedTiles;
}
struct ScoreEntry {
    int score;
    float time;
};

// Check if player has won by capturing enough territory
bool checkWinCondition(int grid[25][40]) {
    int totalBlocks = M * N;
    int wallBlocks = 0;
    int emptyBlocks = 0;
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == 1) wallBlocks++;
            else if (grid[i][j] == 0) emptyBlocks++;
        }
    }
    
    // Win if 95% or more territory is captured (only 5% or less remains empty)
    float capturePercentage = (float)wallBlocks / totalBlocks * 100.0f;
    return capturePercentage >= 95.0f;
}
void updateTop5Scores(int score, float totalGameTime) {
    ScoreEntry entries[6]; // Max 5 old + 1 new
    int count = 0;
    ifstream infile("data/scores.txt");
    if (infile.is_open()) {
        while (count < 5) {
            int s;
            float t;
            char ch;
            infile >> s >> ch >> t; // Reading format: 100 (5.3)
            if (infile.fail()) break;
            infile.ignore(10, '\n'); // Skip to next line
            entries[count].score = s;
            entries[count].time = t;
            count++;
        }
        infile.close();
    }
    entries[count].score = score;
    entries[count].time = totalGameTime;
    count++;
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (entries[j].score > entries[i].score) {
                ScoreEntry temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
    ofstream outfile("data/scores.txt");
    if (!outfile.is_open()) {
        cerr << "Error: Could not open scores.txt for writing" << endl;
        return;
    }
    for (int i = 0; i < count && i < 5; ++i) {
        outfile << entries[i].score << " (" << fixed << setprecision(1) << entries[i].time << ")\n";
    }
    outfile.close();
}
void displayTopScores(RenderWindow* window, Font& font) {
    ScoreEntry entries[5];
    int count = 0;
    ifstream infile("data/scores.txt");
    if (infile.is_open()) {
        while (count < 5) {
            int s;
            float t;
            char ch;
            infile >> s >> ch >> t >> ch; // Read score : time :
            if (infile.fail()) break;
            entries[count].score = s;
            entries[count].time = t;
            infile.ignore(100, '\n'); // Skip rest of line
            count++;
        }
        infile.close();
    }
    string scoreList = "Top Scores:\n";
    if (count == 0) {
        scoreList += "No scores available.\n";
    } else {
        for (int i = 0; i < count; ++i) {
            int minutes = static_cast<int>(entries[i].time) / 60;
            int seconds = static_cast<int>(entries[i].time) % 60;
            string secondsStr = (seconds < 10 ? "0" : "") + to_string(seconds);
            scoreList += to_string(i + 1) + ". " + to_string(entries[i].score) +
                         " (Time: " + to_string(minutes) + ":" + secondsStr + ")\n";
        }
    }
    Text highScoreText;
    highScoreText.setFont(font);
    highScoreText.setCharacterSize(20);
    highScoreText.setFillColor(Color::White);
    highScoreText.setPosition(100, 200);
    highScoreText.setString(scoreList);
    window->draw(highScoreText);
}
Enemy::Enemy() {
    int gx, gy;
    int maxAttempts = 50;
    int attempts = 0;
    int centerX = N / 2; // 20
    int centerY = M / 2; // 12
    int radius = 2;
        while (attempts < maxAttempts) {
            gx = centerX + (rand() % (2 * radius + 1) - radius);
            gy = centerY + (rand() % (2 * radius + 1) - radius);
            if (gx >= 1 && gx < N - 1 && gy >= 1 && gy < M - 1 && grid[gy][gx] == 0) {
                break;
            }
            attempts++;
        }
        if (attempts >= maxAttempts) {
            gx = centerX;
            gy = centerY;
            if (grid[gy][gx] != 0) {
                gx = centerX - 1;
                gy = centerY - 1;
            }
            cout << "Warning: Could not find valid spawn position near center, using fallback (" << gx << "," << gy << ")" << endl;
        }
        x = gx * ts + ts;
        y = gy * ts + ts;
        speed = 1.0f;
        moveType = 0;
        patternTimer = 0.0f;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        dx = static_cast<int>(cos(angle) * 2);
        dy = static_cast<int>(sin(angle) * 2);
        if (dx == 0 && dy == 0) {
            dx = 1;
            dy = 1;
        }
}
void Enemy::move(float deltaTime) {
    deltaTime = min(deltaTime, 0.033f);
    float effectiveSpeed = min(speed, 5.0f);
    if (moveType == 0) {
        int currentGridX = x / ts;
        int currentGridY = y / ts;
        float preciseStepX = dx * effectiveSpeed * deltaTime * 60;
        float preciseStepY = dy * effectiveSpeed * deltaTime * 60;
        int stepX = static_cast<int>(preciseStepX);
        int stepY = static_cast<int>(preciseStepY);
        int newX = x + stepX;
        int newY = y + stepY;
        bool hitWall = false;
        int margin = ts + 5;
        if (newX <= margin) {
            newX = margin;
            dx = abs(dx);
            dy += (rand() % 5 - 2);
            dy = clamp(dy, -2, 2);
            if (dy == 0) dy = (rand() % 2 == 0) ? 1 : -1;
            hitWall = true;
        } else if (newX >= (N - 1) * ts - 5) {
            newX = (N - 1) * ts - 5;
            dx = -abs(dx);
            dy += (rand() % 5 - 2);
            dy = clamp(dy, -2, 2);
            if (dy == 0) dy = (rand() % 2 == 0) ? 1 : -1;
            hitWall = true;
        }
        if (newY <= margin) {
            newY = margin;
            dy = abs(dy);
            dx += (rand() % 5 - 2);
            dx = clamp(dx, -2, 2);
            if (dx == 0) dx = (rand() % 2 == 0) ? 1 : -1;
            hitWall = true;
        } else if (newY >= (M - 1) * ts - 5) {
            newY = (M - 1) * ts - 5;
            dy = -abs(dy);
            dx += (rand() % 5 - 2);
            dx = clamp(dx, -2, 2);
            if (dx == 0) dx = (rand() % 2 == 0) ? 1 : -1;
            hitWall = true;
        }
        int newGridX = newX / ts;
        int newGridY = newY / ts;
        bool canMove = true;
        if (newGridX >= 0 && newGridX < N && newGridY >= 0 && newGridY < M) {
            if (grid[newGridY][newGridX] == 1) {
                canMove = false;
                if (newGridX != currentGridX) {
                    dx = -dx;
                    dy += (rand() % 3 - 1);
                }
                if (newGridY != currentGridY) {
                    dy = -dy;
                    dx += (rand() % 3 - 1);
                }
                dx = clamp(dx, -2, 2);
                dy = clamp(dy, -2, 2);
                if (dx == 0) dx = (rand() % 2 == 0) ? 1 : -1;
                if (dy == 0) dy = (rand() % 2 == 0) ? 1 : -1;
            }
        }
        if (canMove && !hitWall) {
            x = newX;
            y = newY;
        } else if (hitWall) {
            x = newX;
            y = newY;
        }
        x = clamp(x, margin, (N - 1) * ts - 5);
        y = clamp(y, margin, (M - 1) * ts - 5);
    } else if (moveType == 1) {
        moveZigZag(*this, deltaTime);
    } else if (moveType == 2) {
        moveDriftingSpiral(*this, deltaTime);
    }
}
void moveZigZag(Enemy& enemy, float deltaTime) {
    deltaTime = deltaTime < 0.1f ? deltaTime : 0.1f;
    enemy.speed = enemy.speed < 5.0f ? enemy.speed : 5.0f;
    int stepX = static_cast<int>(enemy.dx * enemy.speed * deltaTime * 60);
    int stepY = static_cast<int>(enemy.dy * enemy.speed * deltaTime * 60);
    int newX = enemy.x + stepX;
    int newY = enemy.y + stepY;
    int margin = ts + 5;
    if (newX <= margin || newX >= (N - 1) * ts - 5) {
        enemy.dx = -enemy.dx;
        enemy.dy += (rand() % 3 - 1);
        enemy.dy = clamp(enemy.dy, -2, 2);
        if (enemy.dy == 0) enemy.dy = (rand() % 2 == 0) ? 1 : -1;
        newX = clamp(newX, margin, (N - 1) * ts - 5);
    }
    if (newY <= margin || newY >= (M - 1) * ts - 5) {
        enemy.dy = -enemy.dy;
        enemy.dx += (rand() % 3 - 1);
        enemy.dx = clamp(enemy.dx, -2, 2);
        if (enemy.dx == 0) enemy.dx = (rand() % 2 == 0) ? 1 : -1;
        newY = clamp(newY, margin, (M - 1) * ts - 5);
    }
    int gridX = newX / ts;
    int gridY = newY / ts;
    if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M) {
        if (grid[gridY][gridX] == 1) {
            enemy.dx = -enemy.dx;
            enemy.dy = -enemy.dy;
            enemy.dx += (rand() % 3 - 1);
            enemy.dy += (rand() % 3 - 1);
            enemy.dx = clamp(enemy.dx, -2, 2);
            enemy.dy = clamp(enemy.dy, -2, 2);
            if (enemy.dx == 0) enemy.dx = (rand() % 2 == 0) ? 1 : -1;
            if (enemy.dy == 0) enemy.dy = (rand() % 2 == 0) ? 1 : -1;
        } else {
            enemy.x = newX;
            enemy.y = newY;
        }
    }
    enemy.x = clamp(enemy.x, margin, (N - 1) * ts - 5);
    enemy.y = clamp(enemy.y, margin, (M - 1) * ts - 5);
    enemy.patternTimer += deltaTime;
    if (enemy.patternTimer >= 0.5f) {
        int temp = enemy.dx;
        enemy.dx = -enemy.dy;
        enemy.dy = temp;
        enemy.patternTimer = 0.0f;
    }
}
void moveDriftingSpiral(Enemy& enemy, float deltaTime) {
    deltaTime = deltaTime < 0.1f ? deltaTime : 0.1f;
    enemy.speed = enemy.speed < 5.0f ? enemy.speed : 5.0f;
    enemy.patternTimer += deltaTime * enemy.speed;
    int margin = ts + 5;
    int minX = margin;
    int maxX = (N - 1) * ts - 5;
    int minY = margin;
    int maxY = (M - 1) * ts - 5;
    float baseRadius = 15.0f;
    float radiusGrowth = 0.8f;
    float maxRadius = 60.0f;
    float angle = enemy.patternTimer * 2.5f;
    float radius = baseRadius + radiusGrowth * enemy.patternTimer;
    if (radius > maxRadius) {
        radius = baseRadius;
        enemy.patternTimer = 0.0f;
        angle = (rand() % 360) * 3.14159f / 180.0f;
    }
    int newX = static_cast<int>(enemy.x + radius * cos(angle) * deltaTime * 10);
    int newY = static_cast<int>(enemy.y + radius * sin(angle) * deltaTime * 10);
    newX = clamp(newX, minX, maxX);
    newY = clamp(newY, minY, maxY);
    int gridX = newX / ts;
    int gridY = newY / ts;
    if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M && grid[gridY][gridX] != 1) {
        enemy.x = newX;
        enemy.y = newY;
    } else {
        enemy.patternTimer = 0.0f;
        angle = (rand() % 360) * 3.14159f / 180.0f;
    }
}
void drop(int y, int x) {
    if (y < 0 || y >= M || x < 0 || x >= N || grid[y][x] != 0) return;
    grid[y][x] = -1;
    drop(y - 1, x);
    drop(y + 1, x);
    drop(y, x - 1);
    drop(y, x + 1);
}
void SingleGame(RenderWindow* window, int difficulty, int playerID, const string& username) {
    PlayerProfile profile(playerID, username);
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(playerID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::White;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = 0;
        }
    }
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                grid[i][j] = 1;
            }
        }
    }
    Font font;
    if (!font.loadFromFile("assets/Fonts/AlexandriaFLF.ttf")) {
        cout << "Error: Failed to load Fonts/AlexandriaFLF.ttf" << endl;
        window->close();
        return;
    }
    Texture themeBgTexture;
    Sprite themeBgSprite;
    bool hasThemeBackground = false;
    if (equippedTheme && !equippedTheme->backgroundImage.empty()) {
        cout << "Attempting to load background: " << equippedTheme->backgroundImage << endl;
        if (themeBgTexture.loadFromFile(equippedTheme->backgroundImage)) {
            themeBgSprite.setTexture(themeBgTexture);
            float scaleX = 720.0f / themeBgTexture.getSize().x;
            float scaleY = 450.0f / themeBgTexture.getSize().y;
            themeBgSprite.setScale(scaleX, scaleY);
            hasThemeBackground = true;
            cout << "Background loaded successfully!" << endl;
        } else {
            cout << "Failed to load background image!" << endl;
        }
    } else {
        cout << "No background image for this theme (Classic theme)" << endl;
    }
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setOutlineThickness(2);
    scoreText.setPosition(20, 10);
    Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(24);
    timeText.setFillColor(Color::White);
    timeText.setOutlineColor(Color::Black);
    timeText.setOutlineThickness(2);
    timeText.setPosition(600, 10);
    Text moveText;
    moveText.setFont(font);
    moveText.setCharacterSize(24);
    moveText.setFillColor(Color::White);
    moveText.setOutlineColor(Color::Black);
    moveText.setOutlineThickness(2);
    moveText.setPosition(300, 10);
    Text powerUpText;
    powerUpText.setFont(font);
    powerUpText.setCharacterSize(24);
    powerUpText.setFillColor(Color::White);
    powerUpText.setOutlineColor(Color::Black);
    powerUpText.setOutlineThickness(2);
    powerUpText.setPosition(450, 10);
    Text levelText;
    levelText.setFont(font);
    levelText.setCharacterSize(18);
    levelText.setFillColor(Color::Cyan);
    levelText.setOutlineColor(Color::Black);
    levelText.setOutlineThickness(2);
    levelText.setPosition(20, 50);
    Texture t1, t2, t3;
    if (!t1.loadFromFile("assets/images/tiles.png")) {
        cout << "Error: Failed to load images/tiles.png" << endl;
        window->close();
        return;
    }
    if (!t2.loadFromFile("assets/images/gameover.png")) {
        cout << "Error: Failed to load images/gameover.png" << endl;
        window->close();
        return;
    }
    if (!t3.loadFromFile("assets/images/enemy.png")) {
        cout << "Error: Failed to load images/enemy.png" << endl;
        window->close();
        return;
    }
    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);
    if (hasThemeBackground) {
        sTile.setColor(Color(255, 255, 255, 220)); // White with slight transparency
        sEnemy.setColor(Color(255, 50, 50)); // Bright red for enemies
    } else {
        sTile.setColor(themeColor);
        sEnemy.setColor(themeSecondary);
    }
    Enemy a[100];
    int enemyCount = 2; //default
    if (difficulty == 1) enemyCount = 2;
    else if (difficulty == 2) enemyCount = 4;
    else if (difficulty == 3) enemyCount = 6;
    else if (difficulty == 4) enemyCount = 2;
    bool Game = true;
    bool playerWon = false;
    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;
    Clock gameClock;
    float totalPausedTime = 0.0f;
    Clock pauseClock;
    score = 0;
    isPaused = false;
    moveCount = 0;
    bool patternSwitched = false;
    static int rewardCounter = 0; // Tracks double points bonus occurrences
    int powerUpCount = 0; // Tracks number of power-ups in inventory
    bool enemiesPaused = false; // Tracks if enemies are paused
    float pauseTimer = 0.0f; // Tracks duration of enemy pause
    int lastPowerUpScore = 0; // Tracks last score at which a power-up was awarded
    while (window->isOpen()) {
        float deltaTime = 0.0f;
        if (!isPaused) {
            deltaTime = clock.restart().asSeconds();
            timer += deltaTime; // Increment timer for player movement
        }
        if (!enemiesPaused) {
            for (int i = 0; i < enemyCount; i++) {
                a[i].move(deltaTime); // if enemy pause move 
            }
        } else {
            pauseTimer += deltaTime;
            if (pauseTimer >= 3.0f) {
                enemiesPaused = false; // End pause after 3 seconds
                pauseTimer = 0.0f; // Reset timer
            }
        }
        float totalGameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        int totalSeconds = static_cast<int>(totalGameTime);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        if (difficulty == 4) { //continous
            float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
            int newEnemyCount = 2 + static_cast<int>(elapsed / 20) * 2;
            enemyCount = (newEnemyCount < 100) ? newEnemyCount : 100; // Limit to array size 100
        }
        float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        static float lastSpeedUpdate = 0.0f;
        if (elapsed >= 20.0f && elapsed - lastSpeedUpdate >= 20.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].speed < 4.0f) {
                    a[i].speed += 0.5f;
                }
            }
            lastSpeedUpdate = elapsed;
        }
        static float lastDirectionChange = 0.0f;
        if (elapsed - lastDirectionChange >= 5.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    if (rand() % 3 == 0) {
                        a[i].dx += (rand() % 3 - 1);
                        a[i].dy += (rand() % 3 - 1);
                        a[i].dx = clamp(a[i].dx, -2, 2);
                        a[i].dy = clamp(a[i].dy, -2, 2);
                        if (a[i].dx == 0) a[i].dx = (rand() % 2 == 0) ? 1 : -1;
                        if (a[i].dy == 0) a[i].dy = (rand() % 2 == 0) ? 1 : -1;
                    }
                }
            }
            lastDirectionChange = elapsed;
        }
        if (elapsed >= 30.0f && !patternSwitched) {
            for (int i = 0; i < enemyCount / 2; i++) {
                a[i].moveType = 1;  
            }
            for (int i = enemyCount / 2; i < enemyCount; i++) {
                a[i].moveType = 2;
            }
            patternSwitched = true;
        }
        Event e;
        while (window->pollEvent(e)) {
            if (e.type == Event::Closed)
                window->close();
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                isPaused = true;
                pauseClock.restart();
                showPauseMenu(window);
                totalPausedTime += pauseClock.getElapsedTime().asSeconds();
                clock.restart();
                isPaused = false;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::F5) {
                extern SaveGame* g_saveGame;
                if (g_saveGame) {
                    g_saveGame->saveGame(playerID, username, 1, difficulty, grid, x, y, dx, dy,
                                        score, moveCount, powerUpCount, 0, 0, 0, 0, 0, 0,
                                        a, enemyCount, totalGameTime);
                }
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space && !isPaused) {
                if (powerUpCount > 0 && !enemiesPaused) {
                    powerUpCount--; // Consume one power-up
                    enemiesPaused = true; // Pause enemies
                    pauseTimer = 0.0f; // Reset pause timer
                }
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::Left)) { dx = -1; dy = 0; }
        if (Keyboard::isKeyPressed(Keyboard::Right)) { dx = 1; dy = 0; }
        if (Keyboard::isKeyPressed(Keyboard::Up)) { dx = 0; dy = -1; }
        if (Keyboard::isKeyPressed(Keyboard::Down)) { dx = 0; dy = 1; }
        if (!Game) continue;
        if (timer > delay) {
            x += dx;
            y += dy;
            if (x < 0) x = 0; if (x > N - 1) x = N - 1;
            if (y < 0) y = 0; if (y > M - 1) y = M - 1;
            if (grid[y][x] == 2) Game = false; // player hit its own trail
            if (grid[y][x] == 0 && (dx != 0 || dy != 0)) {
                int prevX = x - dx;
                int prevY = y - dy;
                if (prevX >= 0 && prevX < N && prevY >= 0 && prevY < M && grid[prevY][prevX] == 1) {
                    moveCount++;
                }
            }
            if (grid[y][x] == 0) {
                grid[y][x] = 2;
            }
            timer = 0;
        }
        if (grid[y][x] == 1) {
            dx = dy = 0;
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX);
                }
            }
            int capturedTiles = 0;
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) {
                        grid[i][j] = 0; // Reset temporary flood-filled tiles
                    } else if (grid[i][j] == 0 || grid[i][j] == 2) {
                        grid[i][j] = 1; // Convert empty or trail tiles to wall
                        capturedTiles++; // Count each tile in the enclosed area
                    }
                }
            }
            if (capturedTiles > 0) {
                int points = capturedTiles; // Base: 1 point per tile
                int threshold = (rewardCounter >= 3) ? 5 : 10; // Threshold for bonus
                int multiplier = 1;
                if (capturedTiles > threshold) {
                    multiplier = (rewardCounter >= 5) ? 4 : 2; // x4 after 5 occurrences, else x2
                    rewardCounter++; // Increment reward counter for bonus
                }
                points *= multiplier;
                updateScore(points); // Fixed: Single argument
                if (score >= 50 && score > lastPowerUpScore) {
                    if (score <= 100) {
                        if (score >= 50 && lastPowerUpScore < 50) {
                            powerUpCount++;
                            lastPowerUpScore = 50;
                        } else if (score >= 70 && lastPowerUpScore < 70) {
                            powerUpCount++;
                            lastPowerUpScore = 70;
                        } else if (score >= 100 && lastPowerUpScore < 100) {
                            powerUpCount++;
                            lastPowerUpScore = 100;
                        }
                    } else {
                        int excessScore = score - 100;
                        int nextPowerUpScore = 100 + ((excessScore / 30) * 30);
                        if (nextPowerUpScore > lastPowerUpScore) {
                            powerUpCount++;
                            lastPowerUpScore = nextPowerUpScore;
                        }
                    }
                }
            }
        }
        
        // Check win condition after capturing area
        if (checkWinCondition(grid)) {
            Game = false;
            playerWon = true;
            cout << "\n=== VICTORY! ===" << endl;
            cout << "You've captured 95% of the territory!" << endl;
        }
        
        for (int i = 0; i < enemyCount; i++) {
            int gridX = a[i].x / ts; 
            int gridY = a[i].y / ts;
            if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M && grid[gridY][gridX] == 2) {
                Game = false; // game stops if the enemy hit the player trail
            }
        }
        window->clear(enemiesPaused ? Color(100, 0, 0) : (hasThemeBackground ? Color::Black : themeColor));
        if (hasThemeBackground && !enemiesPaused) {
            window->draw(themeBgSprite);
        }
        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue;
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts));
                if (grid[i][j] == 2) sTile.setTextureRect(IntRect(54, 0, ts, ts));
                sTile.setPosition(j * ts, i * ts);
                window->draw(sTile);
            }
        sTile.setTextureRect(IntRect(36, 0, ts, ts));
        sTile.setPosition(x * ts, y * ts);
        window->draw(sTile);
        scoreText.setString("Score: " + to_string(score));
        window->draw(scoreText);
        string timeStr = "Time: " + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds);
        timeText.setString(timeStr);
        window->draw(timeText);
        moveText.setString("Moves: " + to_string(moveCount));
        window->draw(moveText);
        powerUpText.setString("Power-Ups: " + to_string(powerUpCount));
        window->draw(powerUpText);
        levelText.setString("Lvl " + to_string(profile.getCurrentLevel()) + " | XP: " + to_string(profile.getCurrentXP()) + "/" + to_string(profile.getXPToNextLevel()));
        window->draw(levelText);
        sEnemy.rotate(100);
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y);
            window->draw(sEnemy);
        }
        if (!Game) {
            if (playerWon) {
                // Display victory message
                Text winText;
                winText.setFont(font);
                winText.setCharacterSize(48);
                winText.setFillColor(Color::Green);
                winText.setOutlineColor(Color::Black);
                winText.setOutlineThickness(3);
                winText.setString("VICTORY!");
                winText.setPosition(250, 150);
                window->draw(winText);
                
                Text winSubText;
                winSubText.setFont(font);
                winSubText.setCharacterSize(24);
                winSubText.setFillColor(Color::White);
                winSubText.setString("You captured 95% of the territory!");
                winSubText.setPosition(150, 220);
                window->draw(winSubText);
            } else {
                window->draw(sGameover);
            }
            updateTop5Scores(score, totalGameTime); // update score  and time to the file 
            MinHeapLeaderboard leaderboard;
            leaderboard.insertOrUpdate(playerID, username, score);
            profile.addPoints(score);
            showEndMenu(window, score);
        }
        window->display();
    }
}

void SingleGame(RenderWindow* window, int difficulty, int playerID, const string& username,
                int loadedGrid[25][40], int startX, int startY, int startDX, int startDY,
                int startScore, int startMoves, int startPowerUps,
                Enemy* loadedEnemies, int loadedEnemyCount, float startTime, int saveID = -1) {
    
    PlayerProfile profile(playerID, username);
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(playerID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::White;
    
    // Copy loaded grid state
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = loadedGrid[i][j];
        }
    }
    
    Font font;
    if (!font.loadFromFile("assets/Fonts/AlexandriaFLF.ttf")) {
        cout << "Error: Failed to load Fonts/AlexandriaFLF.ttf" << endl;
        window->close();
        return;
    }
    
    Texture themeBgTexture;
    Sprite themeBgSprite;
    bool hasThemeBackground = false;
    if (equippedTheme && !equippedTheme->backgroundImage.empty()) {
        if (themeBgTexture.loadFromFile(equippedTheme->backgroundImage)) {
            themeBgSprite.setTexture(themeBgTexture);
            float scaleX = 720.0f / themeBgTexture.getSize().x;
            float scaleY = 450.0f / themeBgTexture.getSize().y;
            themeBgSprite.setScale(scaleX, scaleY);
            hasThemeBackground = true;
        }
    }
    
    Text scoreText, timeText, moveText, powerUpText, levelText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setOutlineThickness(2);
    scoreText.setPosition(20, 10);
    
    timeText.setFont(font);
    timeText.setCharacterSize(24);
    timeText.setFillColor(Color::White);
    timeText.setOutlineColor(Color::Black);
    timeText.setOutlineThickness(2);
    timeText.setPosition(600, 10);
    
    moveText.setFont(font);
    moveText.setCharacterSize(24);
    moveText.setFillColor(Color::White);
    moveText.setOutlineColor(Color::Black);
    moveText.setOutlineThickness(2);
    moveText.setPosition(300, 10);
    
    powerUpText.setFont(font);
    powerUpText.setCharacterSize(24);
    powerUpText.setFillColor(Color::White);
    powerUpText.setOutlineColor(Color::Black);
    powerUpText.setOutlineThickness(2);
    powerUpText.setPosition(450, 10);
    
    levelText.setFont(font);
    levelText.setCharacterSize(18);
    levelText.setFillColor(Color::Cyan);
    levelText.setOutlineColor(Color::Black);
    levelText.setOutlineThickness(1);
    levelText.setPosition(20, 40);
    
    Texture t1, t2, t3;
    if (!t1.loadFromFile("assets/images/tiles.png") || 
        !t2.loadFromFile("assets/images/gameover.png") ||
        !t3.loadFromFile("assets/images/enemy.png")) {
        cout << "Error loading images!" << endl;
        window->close();
        return;
    }
    
    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);
    
    if (hasThemeBackground) {
        sTile.setColor(Color(255, 255, 255, 220));
        sEnemy.setColor(Color(255, 50, 50));
    } else {
        sTile.setColor(themeColor);
        sEnemy.setColor(themeSecondary);
    }
    
    // Copy loaded enemies
    Enemy a[100];
    int enemyCount = loadedEnemyCount;
    for (int i = 0; i < enemyCount; i++) {
        a[i] = loadedEnemies[i];
    }
    
    // Initialize from loaded state
    bool Game = true;
    bool playerWon = false;
    int x = startX, y = startY, dx = startDX, dy = startDY;
    float timer = 0, delay = 0.07;
    Clock clock;
    Clock gameClock;
    float totalPausedTime = 0.0f;
    Clock pauseClock;
    
    score = startScore;
    isPaused = false;
    moveCount = startMoves;
    bool patternSwitched = false;
    int powerUpCount = startPowerUps;
    bool enemiesPaused = false;
    float pauseTimer = 0.0f;
    int lastPowerUpScore = (startScore / 500) * 500;
    
    // Adjust game clock to match loaded time
    gameClock.restart();
    totalPausedTime = -startTime;  // Negative so when added it gives correct elapsed time
    
    MinHeapLeaderboard leaderboard;
    
    // Main game loop - same as original but starts from loaded state
    while (window->isOpen()) {
        float deltaTime = 0.0f;
        if (!isPaused) {
            deltaTime = clock.restart().asSeconds();
            timer += deltaTime;
        }
        
        if (!enemiesPaused) {
            for (int i = 0; i < enemyCount; i++) {
                a[i].move(deltaTime);
            }
        } else {
            pauseTimer += deltaTime;
            if (pauseTimer >= 3.0f) {
                enemiesPaused = false;
                pauseTimer = 0.0f;
            }
        }
        
        float totalGameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        int totalSeconds = static_cast<int>(totalGameTime);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        
        if (difficulty == 4) {
            float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
            int newEnemyCount = 2 + static_cast<int>(elapsed / 20) * 2;
            enemyCount = (newEnemyCount < 100) ? newEnemyCount : 100;
        }
        
        float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        static float lastSpeedUpdate = 0.0f;
        if (elapsed >= 20.0f && elapsed - lastSpeedUpdate >= 20.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].speed < 4.0f) {
                    a[i].speed += 0.5f;
                }
            }
            lastSpeedUpdate = elapsed;
        }
        
        static float lastDirectionChange = 0.0f;
        if (elapsed - lastDirectionChange >= 5.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    if (rand() % 3 == 0) {
                        a[i].dx += (rand() % 3 - 1);
                        a[i].dy += (rand() % 3 - 1);
                        a[i].dx = clamp(a[i].dx, -2, 2);
                        a[i].dy = clamp(a[i].dy, -2, 2);
                        if (a[i].dx == 0) a[i].dx = (rand() % 2 == 0) ? 1 : -1;
                        if (a[i].dy == 0) a[i].dy = (rand() % 2 == 0) ? 1 : -1;
                    }
                }
            }
            lastDirectionChange = elapsed;
        }
        
        if (elapsed >= 30.0f && !patternSwitched) {
            for (int i = 0; i < enemyCount / 2; i++) {
                a[i].moveType = 1;
            }
            for (int i = enemyCount / 2; i < enemyCount; i++) {
                a[i].moveType = 2;
            }
            patternSwitched = true;
        }
        
        Event e;
        while (window->pollEvent(e)) {
            if (e.type == Event::Closed)
                window->close();
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                isPaused = true;
                pauseClock.restart();
                showPauseMenu(window);
                totalPausedTime += pauseClock.getElapsedTime().asSeconds();
                clock.restart();
                isPaused = false;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::F5) {
                extern SaveGame* g_saveGame;
                if (g_saveGame) {
                    g_saveGame->saveGame(playerID, username, 1, difficulty, grid, x, y, dx, dy,
                                        score, moveCount, powerUpCount, 0, 0, 0, 0, 0, 0,
                                        a, enemyCount, totalGameTime);
                }
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space && !isPaused) {
                if (powerUpCount > 0 && !enemiesPaused) {
                    powerUpCount--;
                    enemiesPaused = true;
                    pauseTimer = 0.0f;
                }
            }
        }
        
        if (Keyboard::isKeyPressed(Keyboard::Left)) { dx = -1; dy = 0; }
        if (Keyboard::isKeyPressed(Keyboard::Right)) { dx = 1; dy = 0; }
        if (Keyboard::isKeyPressed(Keyboard::Up)) { dx = 0; dy = -1; }
        if (Keyboard::isKeyPressed(Keyboard::Down)) { dx = 0; dy = 1; }
        
        if (!Game) continue;
        
        if (timer > delay) {
            x += dx;
            y += dy;
            if (x < 0) x = 0; if (x > N - 1) x = N - 1;
            if (y < 0) y = 0; if (y > M - 1) y = M - 1;
            
            if (grid[y][x] == 2) Game = false;
            
            if (grid[y][x] == 0 && (dx != 0 || dy != 0)) {
                int prevX = x - dx;
                int prevY = y - dy;
                if (prevX >= 0 && prevX < N && prevY >= 0 && prevY < M && grid[prevY][prevX] == 1) {
                    moveCount++;
                }
            }
            
            if (grid[y][x] == 0) {
                grid[y][x] = 2;
            } else if (grid[y][x] == 1) {
                dx = dy = 0;
                for (int i = 0; i < M; i++)
                    for (int j = 0; j < N; j++)
                        if (grid[i][j] == 2) grid[i][j] = 1;
                
                for (int i = 0; i < enemyCount; i++) {
                    int dropY = a[i].y / ts;
                    int dropX = a[i].x / ts;
                    if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                        drop(dropY, dropX);
                    }
                }
                
                int captured = 0;
                for (int i = 0; i < M; i++) {
                    for (int j = 0; j < N; j++) {
                        if (grid[i][j] == -1) {
                            grid[i][j] = 0;
                        } else if (grid[i][j] == 0 || grid[i][j] == 2) {
                            grid[i][j] = 1;
                            captured++;
                        }
                    }
                }
                
                updateScore(captured);
                
                if (score >= lastPowerUpScore + 500) {
                    powerUpCount++;
                    lastPowerUpScore = (score / 500) * 500;
                }
                
                // Check win condition after capturing area
                if (checkWinCondition(grid)) {
                    Game = false;
                    playerWon = true;
                    cout << "\n=== VICTORY! ===";
                    cout << "\nYou've captured 95% of the territory!" << endl;
                }
            }
            timer = 0;
        }
        
        for (int i = 0; i < enemyCount; i++) {
            int ex = a[i].x / ts;
            int ey = a[i].y / ts;
            if (ex >= 0 && ex < N && ey >= 0 && ey < M) {
                if (grid[ey][ex] == 2) Game = false;
            }
            if (ex == x && ey == y) Game = false;
        }
        
        scoreText.setString("Score: " + to_string(score));
        moveText.setString("Moves: " + to_string(moveCount));
        powerUpText.setString("Power-Ups: " + to_string(powerUpCount));
        levelText.setString("Lvl " + to_string(profile.getCurrentLevel()) + " | XP: " + to_string(profile.getCurrentXP()) + "/" + to_string(profile.getXPToNextLevel()));
        
        char timeBuffer[32];
        snprintf(timeBuffer, sizeof(timeBuffer), "Time: %02d:%02d", minutes, seconds);
        timeText.setString(timeBuffer);
        
        window->clear(enemiesPaused ? Color(100, 0, 0) : (hasThemeBackground ? Color::Black : themeColor));
        if (hasThemeBackground && !enemiesPaused) {
            window->draw(themeBgSprite);
        }
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue;
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts));
                if (grid[i][j] == 2) sTile.setTextureRect(IntRect(54, 0, ts, ts));
                sTile.setPosition(j * ts, i * ts);
                window->draw(sTile);
            }
        }
        
        sTile.setTextureRect(IntRect(36, 0, ts, ts));
        sTile.setPosition(x * ts, y * ts);
        window->draw(sTile);
        
        window->draw(scoreText);
        window->draw(timeText);
        window->draw(moveText);
        window->draw(powerUpText);
        window->draw(levelText);
        
        sEnemy.rotate(100);
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y);
            window->draw(sEnemy);
        }
        
        if (!Game) {
            if (playerWon) {
                // Display victory message
                Text winText;
                winText.setFont(font);
                winText.setCharacterSize(48);
                winText.setFillColor(Color::Green);
                winText.setOutlineColor(Color::Black);
                winText.setOutlineThickness(3);
                winText.setString("VICTORY!");
                winText.setPosition(250, 150);
                window->draw(winText);
                
                Text winSubText;
                winSubText.setFont(font);
                winSubText.setCharacterSize(24);
                winSubText.setFillColor(Color::White);
                winSubText.setString("You captured 95% of the territory!");
                winSubText.setPosition(150, 220);
                window->draw(winSubText);
            } else {
                window->draw(sGameover);
            }
            leaderboard.insertOrUpdate(playerID, username, score);
            profile.addPoints(score);
            
            // Delete save file if this was a loaded game
            if (saveID != -1) {
                extern SaveGame* g_saveGame;
                if (g_saveGame && g_saveGame->deleteSave(saveID)) {
                    cout << "Save file deleted (player died)" << endl;
                }
            }
            
            showEndMenu(window, score);
        }
        
        window->display();
    }
}

void MultiGame(RenderWindow* window, int difficulty, int p1ID, const string& p1Name, int p2ID, const string& p2Name) {
    PlayerProfile p1Profile(p1ID, p1Name);
    PlayerProfile p2Profile(p2ID, p2Name);
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(p1ID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::White;
    srand(static_cast<unsigned>(time(nullptr)));
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = 0; // Set cell to empty
        }
    }
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                grid[i][j] = 1; // Mark as wall
            }
        }
    }
    Font font;
    if (!font.loadFromFile("assets/Fonts/AlexandriaFLF.ttf")) {
        cout << "Error: Failed to load Fonts/AlexandriaFLF.ttf" << endl;
        window->close();
        return;
    }
    Texture themeBgTexture;
    Sprite themeBgSprite;
    bool hasThemeBackground = false;
    if (equippedTheme && !equippedTheme->backgroundImage.empty()) {
        if (themeBgTexture.loadFromFile(equippedTheme->backgroundImage)) {
            themeBgSprite.setTexture(themeBgTexture);
            float scaleX = 720.0f / themeBgTexture.getSize().x;
            float scaleY = 450.0f / themeBgTexture.getSize().y;
            themeBgSprite.setScale(scaleX, scaleY);
            hasThemeBackground = true;
        }
    }
    Text p1ScoreText;
    p1ScoreText.setFont(font);
    p1ScoreText.setCharacterSize(20);
    p1ScoreText.setFillColor(Color::Red);
    p1ScoreText.setOutlineColor(Color::Black);
    p1ScoreText.setOutlineThickness(2);
    p1ScoreText.setPosition(20, 10.f);
    Text p1PowerUpText;
    p1PowerUpText.setFont(font);
    p1PowerUpText.setCharacterSize(20);
    p1PowerUpText.setFillColor(Color::White);
    p1PowerUpText.setOutlineColor(Color::Black);
    p1PowerUpText.setOutlineThickness(2);
    p1PowerUpText.setPosition(20, 40.f);
    Text p2ScoreText;
    p2ScoreText.setFont(font);
    p2ScoreText.setCharacterSize(20);
    p2ScoreText.setFillColor(Color::White);
    p2ScoreText.setOutlineColor(Color::Black);
    p2ScoreText.setOutlineThickness(2);
    p2ScoreText.setPosition(500, 10.f);
    Text p2PowerUpText;
    p2PowerUpText.setFont(font);
    p2PowerUpText.setCharacterSize(20);
    p2PowerUpText.setFillColor(Color::White);
    p2PowerUpText.setOutlineColor(Color::Black);
    p2PowerUpText.setOutlineThickness(2);
    p2PowerUpText.setPosition(500, 40.f);
    Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(20);
    timeText.setFillColor(Color::Yellow);
    timeText.setPosition(300, 10);
    Text p1LevelText;
    p1LevelText.setFont(font);
    p1LevelText.setCharacterSize(16);
    p1LevelText.setFillColor(Color::Cyan);
    p1LevelText.setPosition(20, 70);
    Text p2LevelText;
    p2LevelText.setFont(font);
    p2LevelText.setCharacterSize(16);
    p2LevelText.setFillColor(Color::Cyan);
    p2LevelText.setPosition(500, 70);
    Texture t1, t2, t3;
    if (!t1.loadFromFile("assets/images/tiles.png")) {
        cout << "Error: Failed to load images/tiles.png" << endl;
        window->close();
        return;
    }
    if (!t2.loadFromFile("assets/images/gameover.png")) {
        cout << "Error: Failed to load images/gameover.png" << endl;
        window->close();
        return;
    }
    if (!t3.loadFromFile("assets/images/enemy.png")) {
        cout << "Error: Failed to load images/enemy.png" << endl;
        window->close();
        return;
    }
    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100); // Center game over sprite
    sEnemy.setOrigin(20, 20); // Set enemy sprite origin to center
    if (hasThemeBackground) {
        sTile.setColor(Color(255, 255, 255, 220)); // White with slight transparency
        sEnemy.setColor(Color(255, 50, 50)); // Bright red for enemies
    } else {
        sTile.setColor(themeColor);
        sEnemy.setColor(themeSecondary);
    }
    Enemy a[100];
    int enemyCount = 2; // Default enemy count
    if (difficulty == 1) enemyCount = 2; // Easy
    else if (difficulty == 2) enemyCount = 4; // Medium
    else if (difficulty == 3) enemyCount = 6; // Hard
    else if (difficulty == 4) enemyCount = 2; // Continous
    else {
        enemyCount = 2; // Fallback default
    }
    for (int i = 0; i < enemyCount; i++) {
        int ex, ey; // Enemy grid coordinates
        int maxAttempts = 50; // Maximum spawn attempts
        int attempts = 0;
        while (attempts < maxAttempts) {
            ex = rand() % (N - 2) + 1; // Avoid border cells
            ey = rand() % (M - 2) + 1;
            if (grid[ey][ex] == 0 && !(ex == 1 && ey == 1) && !(ex == N-2 && ey == M-2)) {
                break;
            }
            attempts++;
        }
        if (attempts >= maxAttempts) {
            ex = N / 2;
            ey = M / 2;
        }
        a[i].x = ex * ts + ts / 2.0f; // Set pixel X, centered in tile
        a[i].y = ey * ts + ts / 2.0f; // Set pixel Y, centered in tile
        a[i].speed = 1.0f; // Initial speed
        a[i].moveType = 0; // Linear movement
        a[i].dx = (rand() % 2 == 0 ? 1 : -1); // Random X direction
        a[i].dy = (rand() % 2 == 0 ? 1 : -1); // Random Y direction
    }
    bool p1Game = true; // Player 1 game state
    int p1x = 1, p1y = 1, p1dx = 0, p1dy = 0; // Position and direction
    int p1Score = 0; // Score
    int p1PowerUpCount = 0; // Power-up count
    int p1LastPowerUpScore = 0; // Last score for power-up award
    int p1MoveCount = 0; // Move counter
    bool p2Game = true; // Player 2 game state
    int p2x = N-2, p2y = M-2, p2dx = 0, p2dy = 0; // Position and direction
    int p2Score = 0; // Score
    int p2PowerUpCount = 0; // Power-up count
    int p2LastPowerUpScore = 0; // Last score for power-up award
    int p2MoveCount = 0; // Move counter
    grid[p1y][p1x] = 3; // Player 1 trail
    grid[p2y][p2x] = 4; // Player 2 trail
    float timer = 0, delay = 0.07; // Movement timer and delay
    Clock clock; // Frame clock
    Clock gameClock; // Game duration clock
    float totalPausedTime = 0.0f; // Total paused time
    Clock pauseClock; // Pause duration clock
    int frameCount = 0; // Frame counter for initial enemy delay
    isPaused = false; // Game pause state
    bool enemiesPaused = false; // Enemy pause state
    float pauseTimer = 0.0f; // Enemy pause duration
    bool p1Paused = false; // Player 1 pause state
    bool p2Paused = false; // Player 2 pause state
    bool patternSwitched = false; // Enemy pattern switch flag
    static int p1RewardCounter = 0; // Tracks bonus occurrences for Player 1
    static int p2RewardCounter = 0; // Tracks bonus occurrences for Player 2
    while (window->isOpen()) {
        float deltaTime = 0.0f;
        if (!isPaused) {
            deltaTime = min(clock.restart().asSeconds(), 0.033f); // Cap at ~60 FPS
            timer += deltaTime; // Update movement timer
            frameCount++; // Increment frame counter
        }
        if (!enemiesPaused && frameCount > 30) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    a[i].move(deltaTime); // Linear movement
                } else if (a[i].moveType == 1) {
                    moveZigZag(a[i], deltaTime); // Zigzag movement
                } else if (a[i].moveType == 2) {
                    moveDriftingSpiral(a[i], deltaTime); // Spiral movement
                }
            }
        } else if (enemiesPaused) {
            pauseTimer += deltaTime; // Update pause timer
            if (pauseTimer >= 3.0f) {
                enemiesPaused = false; // Resume enemies
                p1Paused = false; // Resume Player 1
                p2Paused = false; // Resume Player 2
                pauseTimer = 0.0f; // Reset timer
            }
        }
        float totalGameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        int totalSeconds = static_cast<int>(totalGameTime);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        if (difficulty == 4) {
            float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
            int newEnemyCount = 2 + static_cast<int>(elapsed / 20) * 2;
            enemyCount = min(100, newEnemyCount); // Cap at 100 enemies
        }
        float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        static float lastSpeedUpdate = 0.0f;
        if (elapsed >= 20.0f && elapsed - lastSpeedUpdate >= 20.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].speed < 4.0f) {
                    a[i].speed += 0.5f;
                }
            }
            lastSpeedUpdate = elapsed;
        }
        static float lastDirectionChange = 0.0f;
        if (elapsed - lastDirectionChange >= 5.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    if (rand() % 3 == 0) {
                        a[i].dx += (rand() % 3 - 1);
                        a[i].dy += (rand() % 3 - 1);
                        a[i].dx = clamp(a[i].dx, -2, 2);
                        a[i].dy = clamp(a[i].dy, -2, 2);
                        if (a[i].dx == 0) a[i].dx = (rand() % 2 == 0) ? 1 : -1;
                        if (a[i].dy == 0) a[i].dy = (rand() % 2 == 0) ? 1 : -1;
                    }
                }
            }
            lastDirectionChange = elapsed;
        }
        if (elapsed >= 30.0f && !patternSwitched) {
            for (int i = 0; i < enemyCount / 2; i++) {
                a[i].moveType = 1; // Set to zigzag
            }
            for (int i = enemyCount / 2; i < enemyCount; i++) {
                a[i].moveType = 2; // Set to spiral
            }
            patternSwitched = true; // Prevent re-switching
        }
        Event e;
        while (window->pollEvent(e)) {
            if (e.type == Event::Closed)
                window->close(); // Close window
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Escape) {
                    isPaused = true; // Pause game
                    pauseClock.restart(); // Start pause timer
                    showPauseMenu(window); // Display pause menu
                    totalPausedTime += pauseClock.getElapsedTime().asSeconds();
                    clock.restart(); // Reset frame clock
                    isPaused = false; // Resume game
                }
                if (e.key.code == Keyboard::F5) {
                    extern SaveGame* g_saveGame;
                    if (g_saveGame) {
                        g_saveGame->saveGame(p1ID, p1Name, 2, difficulty, grid, p1x, p1y, p1dx, p1dy,
                                            p1Score, p1MoveCount, p1PowerUpCount, p2x, p2y, p2dx, p2dy,
                                            p2Score, p2PowerUpCount, a, enemyCount, totalGameTime);
                    }
                }
                if (e.key.code == Keyboard::Space && !isPaused && p1Game && p1PowerUpCount > 0) {
                    p1PowerUpCount--; // Decrease power-up count
                    enemiesPaused = true; // Pause enemies
                    p2Paused = true; // Pause Player 2
                    pauseTimer = 0.0f; // Reset pause timer
                }
                if (e.key.code == Keyboard::E && !isPaused && p2Game && p2PowerUpCount > 0) {
                    p2PowerUpCount--; // Decrease power-up count
                    enemiesPaused = true; // Pause enemies
                    p1Paused = true; // Pause Player 1
                    pauseTimer = 0.0f; // Reset pause timer
                }
            }
        }
        if (p1Game && !p1Paused) {
            if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::Numpad4)) { p1dx = -1; p1dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::Numpad6)) { p1dx = 1; p1dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::Numpad8)) { p1dx = 0; p1dy = -1; }
            if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::Numpad2)) { p1dx = 0; p1dy = 1; }
        }
        if (p2Game && !p2Paused) {
            if (Keyboard::isKeyPressed(Keyboard::A)) { p2dx = -1; p2dy = 0; }  // Left
            if (Keyboard::isKeyPressed(Keyboard::D)) { p2dx = 1; p2dy = 0; }   // Right
            if (Keyboard::isKeyPressed(Keyboard::W)) { p2dx = 0; p2dy = -1; }  // Up
            if (Keyboard::isKeyPressed(Keyboard::S)) { p2dx = 0; p2dy = 1; }   // Down
        }
        if (timer > delay) {
            if (p1Game && !p1Paused) {
                int p1newX = p1x + p1dx; // Calculate new X
                int p1newY = p1y + p1dy; // Calculate new Y
                if (p1newX < 0) p1newX = 0; if (p1newX > N - 1) p1newX = N - 1;
                if (p1newY < 0) p1newY = 0; if (p1newY > M - 1) p1newY = M - 1;
                if ((grid[p1newY][p1newX] == 3 && p1MoveCount > 0) || grid[p1newY][p1newX] == 4) {
                    p1Game = false; // End Player 1's game
                    for (int y = 0; y < M; y++) {
                        for (int x = 0; x < N; x++) {
                            if (grid[y][x] == 3) grid[y][x] = 0;
                        }
                    }
                } else {
                    p1x = p1newX; // Update position
                    p1y = p1newY;
                    if (grid[p1y][p1x] == 0 && (p1dx != 0 || p1dy != 0)) {
                        grid[p1y][p1x] = 3; // Mark as Player 1 trail
                        p1MoveCount++; // Increment move count
                    }
                }
            }
            if (p2Game && !p2Paused) {
                int p2newX = p2x + p2dx; // Calculate new X
                int p2newY = p2y + p2dy; // Calculate new Y
                if (p2newX < 0) p2newX = 0; if (p2newX > N - 1) p2newX = N - 1;
                if (p2newY < 0) p2newY = 0; if (p2newY > M - 1) p2newY = M - 1;
                if ((grid[p2newY][p2newX] == 4 && p2MoveCount > 0) || grid[p2newY][p2newX] == 3) {
                    p2Game = false; // End Player 2's game
                    for (int y = 0; y < M; y++) {
                        for (int x = 0; x < N; x++) {
                            if (grid[y][x] == 4) grid[y][x] = 0;
                        }
                    }
                } else {
                    p2x = p2newX; // Update position
                    p2y = p2newY;
                    if (grid[p2y][p2x] == 0 && (p2dx != 0 || p2dy != 0)) {
                        grid[p2y][p2x] = 4; // Mark as Player 2 trail
                        p2MoveCount++; // Increment move count
                    }
                }
            }
            timer = 0; // Reset movement timer
        }
        if (p1Game && grid[p1y][p1x] == 1) {
            p1dx = p1dy = 0; // Stop movement
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX); // Mark reachable empty cells
                }
            }
            int p1CapturedTiles = 0; // Count captured tiles
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) {
                        grid[i][j] = 0; // Reset flood-filled cells
                    } else if (grid[i][j] == 0 || grid[i][j] == 3) {
                        grid[i][j] = 1; // Convert to wall
                        p1CapturedTiles++; // Increment captured count
                    }
                }
            }
            if (p1CapturedTiles > 0) {
                int points = p1CapturedTiles; // Base points
                int threshold = (p1RewardCounter >= 3) ? 5 : 10; // Bonus threshold
                int multiplier = 1; // Default multiplier
                if (p1CapturedTiles > threshold) {
                    multiplier = (p1RewardCounter >= 5) ? 4 : 2; // Apply bonus
                    p1RewardCounter++; // Increment bonus counter
                }
                points *= multiplier; // Apply multiplier
                p1Score += points; // Update score
                if (p1Score >= 50 && p1Score > p1LastPowerUpScore) {
                    if (p1Score <= 100) {
                        if (p1Score >= 50 && p1LastPowerUpScore < 50) {
                            p1PowerUpCount++; // Award power-up
                            p1LastPowerUpScore = 50;
                        } else if (p1Score >= 70 && p1LastPowerUpScore < 70) {
                            p1PowerUpCount++;
                            p1LastPowerUpScore = 70;
                        } else if (p1Score >= 100 && p1LastPowerUpScore < 100) {
                            p1PowerUpCount++;
                            p1LastPowerUpScore = 100;
                        }
                    } else {
                        int excessScore = p1Score - 100;
                        int nextPowerUpScore = 100 + ((excessScore / 30) * 30);
                        if (nextPowerUpScore > p1LastPowerUpScore) {
                            p1PowerUpCount++; // Award power-up
                            p1LastPowerUpScore = nextPowerUpScore;
                        }
                    }
                }
            }
        }
        if (p2Game && grid[p2y][p2x] == 1) {
            p2dx = p2dy = 0; // Stop movement
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX); // Mark reachable empty cells
                }
            }
            int p2CapturedTiles = 0; // Count captured tiles
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == -1) {
                        grid[i][j] = 0; // Reset flood-filled cells
                    } else if (grid[i][j] == 0 || grid[i][j] == 4) {
                        grid[i][j] = 1; // Convert to wall
                        p2CapturedTiles++; // Increment captured count
                    }
                }
            }
            if (p2CapturedTiles > 0) {
                int points = p2CapturedTiles; // Base points
                int threshold = (p2RewardCounter >= 3) ? 5 : 10; // Bonus threshold
                int multiplier = 1; // Default multiplier
                if (p2CapturedTiles > threshold) {
                    multiplier = (p2RewardCounter >= 5) ? 4 : 2; // Apply bonus
                    p2RewardCounter++; // Increment bonus counter
                }
                points *= multiplier; // Apply multiplier
                p2Score += points; // Update score
                if (p2Score >= 50 && p2Score > p2LastPowerUpScore) {
                    if (p2Score <= 100) {
                        if (p2Score >= 50 && p2LastPowerUpScore < 50) {
                            p2PowerUpCount++; // Award power-up
                            p2LastPowerUpScore = 50;
                        } else if (p2Score >= 70 && p2LastPowerUpScore < 70) {
                            p2PowerUpCount++;
                            p2LastPowerUpScore = 70;
                        } else if (p2Score >= 100 && p2LastPowerUpScore < 100) {
                            p2PowerUpCount++;
                            p2LastPowerUpScore = 100;
                        }
                    } else {
                        int excessScore = p2Score - 100;
                        int nextPowerUpScore = 100 + ((excessScore / 30) * 30);
                        if (nextPowerUpScore > p2LastPowerUpScore) {
                            p2PowerUpCount++; // Award power-up
                            p2LastPowerUpScore = nextPowerUpScore;
                        }
                    }
                }
            }
        }
        if (frameCount > 30) {
            for (int i = 0; i < enemyCount; i++) {
                int gridX = a[i].x / ts; // Enemy's grid X
                int gridY = a[i].y / ts; // Enemy's grid Y
                if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M) {
                    if (grid[gridY][gridX] == 3 && p1Game) {
                        p1Game = false; // End Player 1's game
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 3) grid[y][x] = 0;
                            }
                        }
                    }
                    if (grid[gridY][gridX] == 4 && p2Game) {
                        p2Game = false; // End Player 2's game
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 4) grid[y][x] = 0;
                            }
                        }
                    }
                }
                if (p1Game) {
                    float distP1 = sqrt(pow(a[i].x - p1x * ts, 2) + pow(a[i].y - p1y * ts, 2));
                    if (distP1 < ts / 2.0f) {
                        p1Game = false; // End Player 1's game
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 3) grid[y][x] = 0;
                            }
                        }
                    }
                }
                if (p2Game) {
                    float distP2 = sqrt(pow(a[i].x - p2x * ts, 2) + pow(a[i].y - p2y * ts, 2));
                    if (distP2 < ts / 2.0f) {
                        p2Game = false; // End Player 2's game
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 4) grid[y][x] = 0;
                            }
                        }
                    }
                }
            }
        }
        window->clear(enemiesPaused ? Color::Red : (hasThemeBackground ? Color::Black : themeColor));
        if (hasThemeBackground && !enemiesPaused) {
            window->draw(themeBgSprite);
        }
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) continue; // Skip empty cells
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts)); // Wall tile
                if (grid[i][j] == 3) sTile.setTextureRect(IntRect(54, 0, ts, ts)); // Player 1 trail
                if (grid[i][j] == 4) {
                    sTile.setTextureRect(IntRect(54, 0, ts, ts)); // Player 2 trail
                    sTile.setColor(Color(100, 100, 255)); // Blue tint for Player 2
                }
                sTile.setPosition(j * ts, i * ts); // Set tile position
                window->draw(sTile); // Draw tile
                sTile.setColor(Color::White); // Reset color
            }
        }
        if (p1Game) {
            sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player tile
            sTile.setPosition(p1x * ts, p1y * ts); // Player 1 position
            window->draw(sTile); // Draw Player 1
        }
        if (p2Game) {
            sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player tile
            sTile.setColor(Color(100, 100, 255)); // Blue tint for Player 2
            sTile.setPosition(p2x * ts, p2y * ts); // Player 2 position
            window->draw(sTile); // Draw Player 2
            sTile.setColor(Color::White); // Reset color
        }
        sEnemy.rotate(100); // Rotate enemy sprite
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y); // Set enemy position
            window->draw(sEnemy); // Draw enemy
        }
        p1ScoreText.setString("P1 Score: " + to_string(p1Score));
        window->draw(p1ScoreText);
        p2ScoreText.setString("P2 Score: " + to_string(p2Score));
        window->draw(p2ScoreText);
        string timeStr = "Time: " + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds);
        timeText.setString(timeStr); // Format game time
        window->draw(timeText);
        p1PowerUpText.setString("P1 Power-Ups: " + to_string(p1PowerUpCount));
        window->draw(p1PowerUpText);
        p2PowerUpText.setString("P2 Power-Ups: " + to_string(p2PowerUpCount));
        window->draw(p2PowerUpText);
        p1LevelText.setString("Lvl " + to_string(p1Profile.getCurrentLevel()) + "\n" + to_string(p1Profile.getCurrentXP()) + "/" + to_string(p1Profile.getXPToNextLevel()));
        window->draw(p1LevelText);
        p2LevelText.setString("Lvl " + to_string(p2Profile.getCurrentLevel()) + "\n" + to_string(p2Profile.getCurrentXP()) + "/" + to_string(p2Profile.getXPToNextLevel()));
        window->draw(p2LevelText);
        if (!p1Game && !p2Game) {
            window->draw(sGameover); // Show game over screen
            string st; // Winner message
            bool p1Won = false, p2Won = false;
            if (p1Score > p2Score) {
                st = "Player 1 Wins!";
                p1Won = true;
            } else if (p2Score > p1Score) {
                st = "Player 2 Wins!";
                p2Won = true;
            } else {
                st = "It's a Tie!";
            }
            int topScore = max(p1Score, p2Score); // Highest score
            updateTop5Scores(topScore, totalGameTime); // Save to high scores
            MinHeapLeaderboard leaderboard;
            leaderboard.insertOrUpdate(p1ID, p1Name, p1Score);
            leaderboard.insertOrUpdate(p2ID, p2Name, p2Score);
            p1Profile.addPoints(p1Score);
            p2Profile.addPoints(p2Score);
            p1Profile.addMatchResult(p2Name, p1Score, p2Score, p1Won);
            p2Profile.addMatchResult(p1Name, p2Score, p1Score, p2Won);
            window->display(); // Render frame
            showMEndMenu(window, topScore, st); // Show end menu
            return; // Exit function
        }
        window->display();
    }
}

void SingleGameWithSave(RenderWindow* window, int saveID) {
    extern SaveGame* g_saveGame;
    if (!g_saveGame) {
        cout << "Error: SaveGame system not initialized!" << endl;
        return;
    }
    
    int difficulty, gameMode;
    int playerX, playerY, playerDX, playerDY;
    int p2X, p2Y, p2DX, p2DY;
    int enemyCount;
    float gameTime;
    int loadedScore, loadedMoveCount, loadedPowerUpCount;
    int p2Score, p2PowerUpCount;
    Enemy enemies[100];
    int loadedGrid[25][40];
    
    bool loaded = g_saveGame->loadGame(saveID, loadedGrid, playerX, playerY, playerDX, playerDY,
                                       loadedScore, loadedMoveCount, loadedPowerUpCount,
                                       p2X, p2Y, p2DX, p2DY, p2Score, p2PowerUpCount,
                                       enemies, enemyCount, gameTime,
                                       gameMode, difficulty);
    
    if (!loaded) {
        cout << "Failed to load save game!" << endl;
        return;
    }
    
    cout << "=== Game Loaded Successfully ===" << endl;
    cout << "Save ID: " << saveID << endl;
    cout << "Game Mode: " << (gameMode == 1 ? "Single Player" : "Multiplayer") << endl;
    cout << "Difficulty: " << difficulty << endl;
    cout << "Score: " << loadedScore << endl;
    cout << "Player Position: (" << playerX << ", " << playerY << ")" << endl;
    cout << "Enemy Count: " << enemyCount << endl;
    
    // Debug: Check grid state
    int wallCount = 0, trailCount = 0, emptyCount = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (loadedGrid[i][j] == 1) wallCount++;
            else if (loadedGrid[i][j] == 2) trailCount++;
            else if (loadedGrid[i][j] == 0) emptyCount++;
        }
    }
    cout << "Grid Stats - Walls: " << wallCount << ", Trails: " << trailCount << ", Empty: " << emptyCount << endl;
    cout << "Resuming from saved position..." << endl;
    
    if (gameMode == 1) {
        // Call overloaded SingleGame with loaded state
        SingleGame(window, difficulty, g_currentPlayerID, g_currentUsername,
                   loadedGrid, playerX, playerY, playerDX, playerDY,
                   loadedScore, loadedMoveCount, loadedPowerUpCount,
                   enemies, enemyCount, gameTime, saveID);
    } else {
        cout << "Multiplayer save loading not fully implemented yet" << endl;
    }
}

int main() {
    if (!loadMenuSound()) {
    }
    srand(time(0));
    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix");
    window.setFramerateLimit(60);
    g_friendSystem = new FriendSystem();
    g_inventoryMgr = new InventoryManager();
    g_saveGame = new SaveGame();
    Authentication auth;
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Error: Failed to load font!" << endl;
        return -1;
    }
    bool authenticated = false;
    while (window.isOpen() && !authenticated) {
        window.clear(Color::Black);
        Text title("XONIX - AUTHENTICATION", font, 30);
        title.setFillColor(Color::Yellow);
        title.setPosition(180, 100);
        Text option1("1. LOGIN", font, 24);
        option1.setFillColor(Color::White);
        option1.setPosition(250, 200);
        Text option2("2. REGISTER", font, 24);
        option2.setFillColor(Color::White);
        option2.setPosition(250, 240);
        Text option3("Press ESC to Exit", font, 18);
        option3.setFillColor(Color(150, 150, 150));
        option3.setPosition(250, 300);
        window.draw(title);
        window.draw(option1);
        window.draw(option2);
        window.draw(option3);
        window.display();
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return 0;
            }
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape) {
                    window.close();
                    return 0;
                }
                if (event.key.code == Keyboard::Num1) {
                    Event clearEvent;
                    while (window.pollEvent(clearEvent)) { }
                    sf::sleep(sf::milliseconds(100));
                    while (window.pollEvent(clearEvent)) { }
                    cout << "Showing login screen..." << endl;
                    cout.flush();
                    authenticated = auth.showLoginScreen(&window);
                    cout << "Login screen returned: " << (authenticated ? "SUCCESS" : "FAILED") << endl;
                    cout.flush();
                    if (authenticated) {
                        cout << "Breaking out of auth loop..." << endl;
                        cout.flush();
                    }
                }
                if (event.key.code == Keyboard::Num2) {
                    Event clearEvent;
                    while (window.pollEvent(clearEvent)) { }
                    sf::sleep(sf::milliseconds(100));
                    while (window.pollEvent(clearEvent)) { }
                    if (auth.showRegistrationScreen(&window)) {
                        authenticated = auth.showLoginScreen(&window);
                    }
                }
            }
        }
    }
    if (!authenticated) {
        delete g_friendSystem;
        delete g_inventoryMgr;
        delete g_saveGame;
        return 0;
    }
    g_currentPlayerID = auth.getCurrentPlayerID();
    g_currentUsername = auth.getCurrentUsername(g_currentPlayerID);
    g_friendSystem->addPlayer(g_currentPlayerID, g_currentUsername);
    if (window.isOpen()) {
        showMenu(&window);
    }
    delete g_friendSystem;
    delete g_inventoryMgr;
    delete g_saveGame;
    return 0;
}
