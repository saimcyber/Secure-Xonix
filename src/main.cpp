/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Main game logic including Enemy AI, scoring system,
 *              single player and multiplayer game modes, area capture mechanics
 */

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
#include <time.h>


using namespace sf;
using namespace std;

// Custom clamp function for older compilers
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

// Global player info
int g_currentPlayerID = -1;
std::string g_currentUsername = "";

// Global Friend System and Inventory Manager (use pointers to avoid early initialization)
FriendSystem* g_friendSystem = nullptr;
InventoryManager* g_inventoryMgr = nullptr;




  
// ------------------ Scoring Function ------------------ //

void updateScore(int capturedTiles) {
    score += capturedTiles;
}



struct ScoreEntry {
    int score;
    float time;
};

void updateTop5Scores(int score, float totalGameTime) {
    ScoreEntry entries[6]; // Max 5 old + 1 new
    int count = 0;

    // Open file for reading
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

    // Add new score
    entries[count].score = score;
    entries[count].time = totalGameTime;
    count++;

    // Sort in descending order based on score
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (entries[j].score > entries[i].score) {
                ScoreEntry temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }

    // Write top 5 scores to file
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

    // Read scores from file
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

    // Build the display string using std::string
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

    // Set up SFML text
    Text highScoreText;
    highScoreText.setFont(font);
    highScoreText.setCharacterSize(20);
    highScoreText.setFillColor(Color::White);
    highScoreText.setPosition(100, 200);
    highScoreText.setString(scoreList);

    // Draw the text
    window->draw(highScoreText);
}
// ------------------ Enemy Structure ------------------

// Enemy constructor implementation
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

        // Random angle for more natural movement
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        dx = static_cast<int>(cos(angle) * 2);
        dy = static_cast<int>(sin(angle) * 2);
        
        // Ensure non-zero movement
        if (dx == 0 && dy == 0) {
            dx = 1;
            dy = 1;
        }
}

// Enemy move implementation
void Enemy::move(float deltaTime) {
    deltaTime = min(deltaTime, 0.033f);
    
    // Cap speed to prevent overshooting at high speeds
    float effectiveSpeed = min(speed, 3.0f);

    if (moveType == 0) {
        // Current grid position
        int currentGridX = x / ts;
        int currentGridY = y / ts;
        
        // Calculate movement step
        int stepX = static_cast<int>(dx * effectiveSpeed * deltaTime * 60);
        int stepY = static_cast<int>(dy * effectiveSpeed * deltaTime * 60);
        
        // Try to move
        int newX = x + stepX;
        int newY = y + stepY;
        
        // Prevent going into boundary tiles
        if (newX <= ts) {
            newX = ts + 1;
            dx = abs(dx);
            // Add randomness
            if (rand() % 2 == 0) {
                dy += (rand() % 3 - 1);
                dy = clamp(dy, -2, 2);
                if (dy == 0) dy = 1;
            }
        } else if (newX >= (N - 1) * ts) {
            newX = (N - 1) * ts - 1;
            dx = -abs(dx);
            if (rand() % 2 == 0) {
                dy += (rand() % 3 - 1);
                dy = clamp(dy, -2, 2);
                if (dy == 0) dy = 1;
            }
        }
        
        if (newY <= ts) {
            newY = ts + 1;
            dy = abs(dy);
            if (rand() % 2 == 0) {
                dx += (rand() % 3 - 1);
                dx = clamp(dx, -2, 2);
                if (dx == 0) dx = 1;
            }
        } else if (newY >= (M - 1) * ts) {
            newY = (M - 1) * ts - 1;
            dy = -abs(dy);
            if (rand() % 2 == 0) {
                dx += (rand() % 3 - 1);
                dx = clamp(dx, -2, 2);
                if (dx == 0) dx = 1;
            }
        }
        
        int newGridX = newX / ts;
        int newGridY = newY / ts;
        
        // Check if new position is valid (not a wall)
        if (newGridX >= 0 && newGridX < N && newGridY >= 0 && newGridY < M) {
            if (grid[newGridY][newGridX] == 1) {
                // Hit a wall - bounce
                if (newGridX != currentGridX) {
                    dx = -dx;
                }
                if (newGridY != currentGridY) {
                    dy = -dy;
                }
                // Add randomness to break patterns
                if (rand() % 3 == 0) {
                    dx += (rand() % 3 - 1);
                    dy += (rand() % 3 - 1);
                    dx = clamp(dx, -2, 2);
                    dy = clamp(dy, -2, 2);
                    if (dx == 0) dx = (rand() % 2 == 0) ? 1 : -1;
                    if (dy == 0) dy = (rand() % 2 == 0) ? 1 : -1;
                }
            } else {
                // Valid move
                x = newX;
                y = newY;
            }
        }
        
        // Final safety clamp
        x = clamp(x, ts + 1, (N - 1) * ts - 1);
        y = clamp(y, ts + 1, (M - 1) * ts - 1);
    } else if (moveType == 1) {
        moveZigZag(*this, deltaTime);
    } else if (moveType == 2) {
        moveDriftingSpiral(*this, deltaTime);
    }
}

// ------------------ Movement Functions ------------------

void moveZigZag(Enemy& enemy, float deltaTime) {
    deltaTime = deltaTime < 0.1f ? deltaTime : 0.1f;
    enemy.speed = enemy.speed < 5.0f ? enemy.speed : 5.0f;

    enemy.x += static_cast<int>(enemy.dx * enemy.speed * deltaTime * 60);
    enemy.y += static_cast<int>(enemy.dy * enemy.speed * deltaTime * 60);

    int gridX = enemy.x / ts;
    int gridY = enemy.y / ts;
    if (gridX < 0 || gridX >= N || gridY < 0 || gridY >= M) {
        enemy.x = clamp(enemy.x, ts, (N - 1) * ts);
        enemy.y = clamp(enemy.y, ts, (M - 1) * ts);
        gridX = enemy.x / ts;
        gridY = enemy.y / ts;
        enemy.dx = -enemy.dx;
        enemy.dy = -enemy.dy;
    }

    if (grid[gridY][gridX] == 1) {
        enemy.dx = -enemy.dx;
        enemy.x += static_cast<int>(enemy.dx * enemy.speed * deltaTime * 60);
    }
    if (grid[gridY][gridX] == 1) {
        enemy.dy = -enemy.dy;
        enemy.y += static_cast<int>(enemy.dy * enemy.speed * deltaTime * 60);
    }

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

    int minX = ts;
    int maxX = (N - 1) * ts;
    int minY = ts;
    int maxY = (M - 1) * ts;

    float baseRadius = 10.0f;
    float radiusGrowth = 0.5f;
    float maxRadius = 50.0f;
    
    // Calculate spiral movement
    float angle = enemy.patternTimer * 2.0f;
    float radius = baseRadius + radiusGrowth * enemy.patternTimer;
    if (radius > maxRadius) {
        radius = maxRadius;
        enemy.patternTimer = 0.0f;
    }
    
    // Calculate new position
    int newX = static_cast<int>(enemy.x + radius * cos(angle) * deltaTime);
    int newY = static_cast<int>(enemy.y + radius * sin(angle) * deltaTime);
    
    // Clamp to valid game area
    newX = clamp(newX, minX, maxX);
    newY = clamp(newY, minY, maxY);
    
    int gridX = newX / ts;
    int gridY = newY / ts;
    
    // Check bounds and walls
    if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M && grid[gridY][gridX] != 1) {
        enemy.x = newX;
        enemy.y = newY;
    } else {
        // Hit wall or out of bounds - reverse spiral
        enemy.patternTimer = 0.0f;
    }
}

// ------------------ Drop Function ------------------

    // this function check if  area is not enclosed
void drop(int y, int x) {
    if (y < 0 || y >= M || x < 0 || x >= N || grid[y][x] != 0) return;
    grid[y][x] = -1;
    drop(y - 1, x);
    drop(y + 1, x);
    drop(y, x - 1);
    drop(y, x + 1);
}

// ------------------ SingleGame Function ------------------

void SingleGame(RenderWindow* window, int difficulty, int playerID, const string& username) {
    // Create player profile at start
    PlayerProfile profile(playerID, username);
    
    // Get player's equipped theme
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(playerID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::White;
    
    // Clear grid
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = 0;
        }
    }

    // Set borders
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
    
    // Load theme background if available
    Texture themeBgTexture;
    Sprite themeBgSprite;
    bool hasThemeBackground = false;
    if (equippedTheme && !equippedTheme->backgroundImage.empty()) {
        cout << "Attempting to load background: " << equippedTheme->backgroundImage << endl;
        if (themeBgTexture.loadFromFile(equippedTheme->backgroundImage)) {
            themeBgSprite.setTexture(themeBgTexture);
            // Scale background to fill window (720x450)
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
    
    // Apply theme colors to sprites - use brighter colors for visibility over backgrounds
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

        // Move enemies
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
                // Cap speed at 4.0 to prevent excessive speeds
                if (a[i].speed < 4.0f) {
                    a[i].speed += 0.5f;
                }
            }
            lastSpeedUpdate = elapsed;
        }
        
        // Add periodic direction randomization every 5 seconds to prevent loops
        static float lastDirectionChange = 0.0f;
        if (elapsed - lastDirectionChange >= 5.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    // Small chance to add random variation
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


            // to get player in bound 
            if (x < 0) x = 0; if (x > N - 1) x = N - 1;
            if (y < 0) y = 0; if (y > M - 1) y = M - 1;

            if (grid[y][x] == 2) Game = false; // player hit its own trail

            

            // move count  
            if (grid[y][x] == 0 && (dx != 0 || dy != 0)) {
                int prevX = x - dx;
                int prevY = y - dy;
                if (prevX >= 0 && prevX < N && prevY >= 0 && prevY < M && grid[prevY][prevX] == 1) {
                    moveCount++;
                }
            }

            if (grid[y][x] == 0) {
                grid[y][x] = 2;
                //  updateScore(1);
            }

            timer = 0;
        }

        if (grid[y][x] == 1) {
            dx = dy = 0;
               // check the enemy is inside the enclosed area then only fill the border
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX);
                }
            }

            // Count captured tiles and apply reward system
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

            // Apply reward system and check for power-ups
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

                // Award power-ups based on score
                if (score >= 50 && score > lastPowerUpScore) {
                    if (score <= 100) {
                        // Check specific thresholds: 50, 70, 100
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
                        // Check 130 and every 30 points thereafter
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

        for (int i = 0; i < enemyCount; i++) {
            // convert pixel coordinates to grid coordinates
            int gridX = a[i].x / ts; 
            int gridY = a[i].y / ts;

            // Check if enemy is within grid bounds and on a player trail tile
            if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M && grid[gridY][gridX] == 2) {
                Game = false; // game stops if the enemy hit the player trail
            }
        }

        window->clear(enemiesPaused ? Color(100, 0, 0) : (hasThemeBackground ? Color::Black : themeColor));
        
        // Draw theme background if available
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
            window->draw(sGameover);
            updateTop5Scores(score, totalGameTime); // update score  and time to the file 
            
            // Update leaderboard and profile
            MinHeapLeaderboard leaderboard;
            leaderboard.insertOrUpdate(playerID, username, score);
            
            profile.addPoints(score);
            
            showEndMenu(window, score);
        }

        window->display();
    }
}

// ------------------ MultiGame Function ------------------


void MultiGame(RenderWindow* window, int difficulty, int p1ID, const string& p1Name, int p2ID, const string& p2Name) {
    // Create player profiles at start
    PlayerProfile p1Profile(p1ID, p1Name);
    PlayerProfile p2Profile(p2ID, p2Name);
    
    // Get player's equipped theme (use P1's theme for multiplayer)
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(p1ID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::White;
    
    // Seed random number generator for enemy spawning
    srand(static_cast<unsigned>(time(nullptr)));

    // Initialize grid by clearing all cells
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            grid[i][j] = 0; // Set cell to empty
        }
    }

    // Set border cells as walls
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                grid[i][j] = 1; // Mark as wall
            }
        }
    }

    // Load font for UI text
    Font font;
    if (!font.loadFromFile("assets/Fonts/AlexandriaFLF.ttf")) {
        cout << "Error: Failed to load Fonts/AlexandriaFLF.ttf" << endl;
        window->close();
        return;
    }
    
    // Load theme background if available
    Texture themeBgTexture;
    Sprite themeBgSprite;
    bool hasThemeBackground = false;
    if (equippedTheme && !equippedTheme->backgroundImage.empty()) {
        if (themeBgTexture.loadFromFile(equippedTheme->backgroundImage)) {
            themeBgSprite.setTexture(themeBgTexture);
            // Scale background to fill window (720x450)
            float scaleX = 720.0f / themeBgTexture.getSize().x;
            float scaleY = 450.0f / themeBgTexture.getSize().y;
            themeBgSprite.setScale(scaleX, scaleY);
            hasThemeBackground = true;
        }
    }

    // Configure Player 1 score text displayed on left side
    Text p1ScoreText;
    p1ScoreText.setFont(font);
    p1ScoreText.setCharacterSize(20);
    p1ScoreText.setFillColor(Color::Red);
    p1ScoreText.setOutlineColor(Color::Black);
    p1ScoreText.setOutlineThickness(2);
    p1ScoreText.setPosition(20, 10.f);

    // Configure Player 1 power-up count text
    Text p1PowerUpText;
    p1PowerUpText.setFont(font);
    p1PowerUpText.setCharacterSize(20);
    p1PowerUpText.setFillColor(Color::White);
    p1PowerUpText.setOutlineColor(Color::Black);
    p1PowerUpText.setOutlineThickness(2);
    p1PowerUpText.setPosition(20, 40.f);

    // Configure Player 2 score text displayed on right side
    Text p2ScoreText;
    p2ScoreText.setFont(font);
    p2ScoreText.setCharacterSize(20);
    p2ScoreText.setFillColor(Color::White);
    p2ScoreText.setOutlineColor(Color::Black);
    p2ScoreText.setOutlineThickness(2);
    p2ScoreText.setPosition(500, 10.f);

    // Configure Player 2 power-up count text
    Text p2PowerUpText;
    p2PowerUpText.setFont(font);
    p2PowerUpText.setCharacterSize(20);
    p2PowerUpText.setFillColor(Color::White);
    p2PowerUpText.setOutlineColor(Color::Black);
    p2PowerUpText.setOutlineThickness(2);
    p2PowerUpText.setPosition(500, 40.f);

    // Configure game time text displayed in center
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

    // Load textures for tiles, game over screen, and enemies
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

    // Create sprites for tiles, game over screen, and enemies
    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100); // Center game over sprite
    sEnemy.setOrigin(20, 20); // Set enemy sprite origin to center
    
    // Apply theme colors to sprites - use brighter colors for visibility over backgrounds
    if (hasThemeBackground) {
        sTile.setColor(Color(255, 255, 255, 220)); // White with slight transparency
        sEnemy.setColor(Color(255, 50, 50)); // Bright red for enemies
    } else {
        sTile.setColor(themeColor);
        sEnemy.setColor(themeSecondary);
    }

    // Array to hold up to 100 enemies
    Enemy a[100];
    int enemyCount = 2; // Default enemy count

    // Set enemy count based on difficulty level
    if (difficulty == 1) enemyCount = 2; // Easy
    else if (difficulty == 2) enemyCount = 4; // Medium
    else if (difficulty == 3) enemyCount = 6; // Hard
    else if (difficulty == 4) enemyCount = 2; // Continous
    else {
        enemyCount = 2; // Fallback default
    }

    // Initialize enemies at random positions away from players
    for (int i = 0; i < enemyCount; i++) {
        int ex, ey; // Enemy grid coordinates
        int maxAttempts = 50; // Maximum spawn attempts
        int attempts = 0;
        // Try to find a valid spawn position
        while (attempts < maxAttempts) {
            ex = rand() % (N - 2) + 1; // Avoid border cells
            ey = rand() % (M - 2) + 1;
            // Ensure cell is empty and not at player starting positions
            if (grid[ey][ex] == 0 && !(ex == 1 && ey == 1) && !(ex == N-2 && ey == M-2)) {
                break;
            }
            attempts++;
        }
        // Use fallback position if no valid spot found
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

    // Player 1 variables (starts at top-left)
    bool p1Game = true; // Player 1 game state
    int p1x = 1, p1y = 1, p1dx = 0, p1dy = 0; // Position and direction
    int p1Score = 0; // Score
    int p1PowerUpCount = 0; // Power-up count
    int p1LastPowerUpScore = 0; // Last score for power-up award
    int p1MoveCount = 0; // Move counter

    // Player 2 variables (starts at bottom-right)
    bool p2Game = true; // Player 2 game state
    int p2x = N-2, p2y = M-2, p2dx = 0, p2dy = 0; // Position and direction
    int p2Score = 0; // Score
    int p2PowerUpCount = 0; // Power-up count
    int p2LastPowerUpScore = 0; // Last score for power-up award
    int p2MoveCount = 0; // Move counter

    // Mark initial player positions on grid
    grid[p1y][p1x] = 3; // Player 1 trail
    grid[p2y][p2x] = 4; // Player 2 trail

    // Timing variables
    float timer = 0, delay = 0.07; // Movement timer and delay
    Clock clock; // Frame clock
    Clock gameClock; // Game duration clock
    float totalPausedTime = 0.0f; // Total paused time
    Clock pauseClock; // Pause duration clock
    int frameCount = 0; // Frame counter for initial enemy delay

    // Game state flags
    isPaused = false; // Game pause state
    bool enemiesPaused = false; // Enemy pause state
    float pauseTimer = 0.0f; // Enemy pause duration
    bool p1Paused = false; // Player 1 pause state
    bool p2Paused = false; // Player 2 pause state
    bool patternSwitched = false; // Enemy pattern switch flag
    static int p1RewardCounter = 0; // Tracks bonus occurrences for Player 1
    static int p2RewardCounter = 0; // Tracks bonus occurrences for Player 2

    // Main game loop
    while (window->isOpen()) {
        // Calculate delta time if not paused
        float deltaTime = 0.0f;
        if (!isPaused) {
            deltaTime = min(clock.restart().asSeconds(), 0.033f); // Cap at ~60 FPS
            timer += deltaTime; // Update movement timer
            frameCount++; // Increment frame counter
        }

        // Move enemies if not paused and after 30-frame delay
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

        // Calculate total game time excluding pauses
        float totalGameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        int totalSeconds = static_cast<int>(totalGameTime);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        // Increase enemy count in continous mode
        if (difficulty == 4) {
            float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
            int newEnemyCount = 2 + static_cast<int>(elapsed / 20) * 2;
            enemyCount = min(100, newEnemyCount); // Cap at 100 enemies
        }

        // Increase enemy speed every 20 seconds
        float elapsed = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
        static float lastSpeedUpdate = 0.0f;
        if (elapsed >= 20.0f && elapsed - lastSpeedUpdate >= 20.0f) {
            for (int i = 0; i < enemyCount; i++) {
                // Cap speed at 4.0 to prevent excessive speeds
                if (a[i].speed < 4.0f) {
                    a[i].speed += 0.5f;
                }
            }
            lastSpeedUpdate = elapsed;
        }
        
        // Add periodic direction randomization every 5 seconds to prevent loops
        static float lastDirectionChange = 0.0f;
        if (elapsed - lastDirectionChange >= 5.0f) {
            for (int i = 0; i < enemyCount; i++) {
                if (a[i].moveType == 0) {
                    // Small chance to add random variation
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

        // Switch enemy movement patterns after 30 seconds
        if (elapsed >= 30.0f && !patternSwitched) {
            for (int i = 0; i < enemyCount / 2; i++) {
                a[i].moveType = 1; // Set to zigzag
            }
            for (int i = enemyCount / 2; i < enemyCount; i++) {
                a[i].moveType = 2; // Set to spiral
            }
            patternSwitched = true; // Prevent re-switching
        }

        // Handle window and keyboard events
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
                // Player 1 uses power-up (Space key)
                if (e.key.code == Keyboard::Space && !isPaused && p1Game && p1PowerUpCount > 0) {
                    p1PowerUpCount--; // Decrease power-up count
                    enemiesPaused = true; // Pause enemies
                    p2Paused = true; // Pause Player 2
                    pauseTimer = 0.0f; // Reset pause timer
                }
                // Player 2 uses power-up (E key)
                if (e.key.code == Keyboard::E && !isPaused && p2Game && p2PowerUpCount > 0) {
                    p2PowerUpCount--; // Decrease power-up count
                    enemiesPaused = true; // Pause enemies
                    p1Paused = true; // Pause Player 1
                    pauseTimer = 0.0f; // Reset pause timer
                }
            }
        }

        // Handle Player 1 input (arrow keys or numpad) if not paused
        if (p1Game && !p1Paused) {
            if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::Numpad4)) { p1dx = -1; p1dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::Numpad6)) { p1dx = 1; p1dy = 0; }
            if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::Numpad8)) { p1dx = 0; p1dy = -1; }
            if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::Numpad2)) { p1dx = 0; p1dy = 1; }
        }

        // Handle Player 2 input (WASD keys) if not paused

        if (p2Game && !p2Paused) {
            if (Keyboard::isKeyPressed(Keyboard::A)) { p2dx = -1; p2dy = 0; }  // Left
            if (Keyboard::isKeyPressed(Keyboard::D)) { p2dx = 1; p2dy = 0; }   // Right
            if (Keyboard::isKeyPressed(Keyboard::W)) { p2dx = 0; p2dy = -1; }  // Up
            if (Keyboard::isKeyPressed(Keyboard::S)) { p2dx = 0; p2dy = 1; }   // Down
        }

        // Update player positions at fixed intervals
        if (timer > delay) {
            // Update Player 1
            if (p1Game && !p1Paused) {
                int p1newX = p1x + p1dx; // Calculate new X
                int p1newY = p1y + p1dy; // Calculate new Y
                // Keep within grid bounds
                if (p1newX < 0) p1newX = 0; if (p1newX > N - 1) p1newX = N - 1;
                if (p1newY < 0) p1newY = 0; if (p1newY > M - 1) p1newY = M - 1;
                // Check for collision with own or opponent's trail
                if ((grid[p1newY][p1newX] == 3 && p1MoveCount > 0) || grid[p1newY][p1newX] == 4) {
                    p1Game = false; // End Player 1's game
                    // Clear Player 1's trail
                    for (int y = 0; y < M; y++) {
                        for (int x = 0; x < N; x++) {
                            if (grid[y][x] == 3) grid[y][x] = 0;
                        }
                    }
                } else {
                    p1x = p1newX; // Update position
                    p1y = p1newY;
                    // Lay trail if moving to empty cell
                    if (grid[p1y][p1x] == 0 && (p1dx != 0 || p1dy != 0)) {
                        grid[p1y][p1x] = 3; // Mark as Player 1 trail
                        p1MoveCount++; // Increment move count
                    }
                }
            }

            // Update Player 2
            if (p2Game && !p2Paused) {
                int p2newX = p2x + p2dx; // Calculate new X
                int p2newY = p2y + p2dy; // Calculate new Y
                // Keep within grid bounds
                if (p2newX < 0) p2newX = 0; if (p2newX > N - 1) p2newX = N - 1;
                if (p2newY < 0) p2newY = 0; if (p2newY > M - 1) p2newY = M - 1;
                // Check for collision with own or opponent's trail
                if ((grid[p2newY][p2newX] == 4 && p2MoveCount > 0) || grid[p2newY][p2newX] == 3) {
                    p2Game = false; // End Player 2's game
                    // Clear Player 2's trail
                    for (int y = 0; y < M; y++) {
                        for (int x = 0; x < N; x++) {
                            if (grid[y][x] == 4) grid[y][x] = 0;
                        }
                    }
                } else {
                    p2x = p2newX; // Update position
                    p2y = p2newY;
                    // Lay trail if moving to empty cell
                    if (grid[p2y][p2x] == 0 && (p2dx != 0 || p2dy != 0)) {
                        grid[p2y][p2x] = 4; // Mark as Player 2 trail
                        p2MoveCount++; // Increment move count
                    }
                }
            }

            timer = 0; // Reset movement timer
        }

        // Handle Player 1 wall collision and area capture
        if (p1Game && grid[p1y][p1x] == 1) {
            p1dx = p1dy = 0; // Stop movement
            // Flood-fill from enemy positions
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX); // Mark reachable empty cells
                }
            }
            int p1CapturedTiles = 0; // Count captured tiles
            // Process grid to capture area
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
            // Award points and power-ups
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
                // Award power-ups based on score milestones
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

        // Handle Player 2 wall collision and area capture
        if (p2Game && grid[p2y][p2x] == 1) {
            p2dx = p2dy = 0; // Stop movement
            // Flood-fill from enemy positions
            for (int i = 0; i < enemyCount; i++) {
                int dropY = a[i].y / ts;
                int dropX = a[i].x / ts;
                if (dropY >= 0 && dropY < M && dropX >= 0 && dropX < N) {
                    drop(dropY, dropX); // Mark reachable empty cells
                }
            }
            int p2CapturedTiles = 0; // Count captured tiles
            // Process grid to capture area
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
            // Award points and power-ups
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
                // Award power-ups based on score milestones
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

        // Check for enemy collisions after initial delay
        if (frameCount > 30) {
            for (int i = 0; i < enemyCount; i++) {
                int gridX = a[i].x / ts; // Enemy's grid X
                int gridY = a[i].y / ts; // Enemy's grid Y
                // Check if enemy hits player trails
                if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M) {
                    if (grid[gridY][gridX] == 3 && p1Game) {
                        p1Game = false; // End Player 1's game
                        // Clear Player 1's trail
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 3) grid[y][x] = 0;
                            }
                        }
                    }
                    if (grid[gridY][gridX] == 4 && p2Game) {
                        p2Game = false; // End Player 2's game
                        // Clear Player 2's trail
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 4) grid[y][x] = 0;
                            }
                        }
                    }
                }
                // Check direct collisions with players
                if (p1Game) {
                    float distP1 = sqrt(pow(a[i].x - p1x * ts, 2) + pow(a[i].y - p1y * ts, 2));
                    if (distP1 < ts / 2.0f) {
                        p1Game = false; // End Player 1's game
                        // Clear Player 1's trail
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
                        // Clear Player 2's trail
                        for (int y = 0; y < M; y++) {
                            for (int x = 0; x < N; x++) {
                                if (grid[y][x] == 4) grid[y][x] = 0;
                            }
                        }
                    }
                }
            }
        }

        // Clear window for rendering
        window->clear(enemiesPaused ? Color::Red : (hasThemeBackground ? Color::Black : themeColor));
        
        // Draw theme background if available
        if (hasThemeBackground && !enemiesPaused) {
            window->draw(themeBgSprite);
        }

        // Draw grid (walls and trails)
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

        // Draw Player 1 if active
        if (p1Game) {
            sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player tile
            sTile.setPosition(p1x * ts, p1y * ts); // Player 1 position
            window->draw(sTile); // Draw Player 1
        }

        // Draw Player 2 if active
        if (p2Game) {
            sTile.setTextureRect(IntRect(36, 0, ts, ts)); // Player tile
            sTile.setColor(Color(100, 100, 255)); // Blue tint for Player 2
            sTile.setPosition(p2x * ts, p2y * ts); // Player 2 position
            window->draw(sTile); // Draw Player 2
            sTile.setColor(Color::White); // Reset color
        }

        // Draw enemies with rotation
        sEnemy.rotate(100); // Rotate enemy sprite
        for (int i = 0; i < enemyCount; i++) {
            sEnemy.setPosition(a[i].x, a[i].y); // Set enemy position
            window->draw(sEnemy); // Draw enemy
        }

        // Update and draw UI elements
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

        // Handle game over for both players
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
            
            // Update leaderboard and profiles for both players
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

        // Display rendered frame
        window->display();
    }
}
// ------------------ Main Function ------------------

int main() {

    cout << "Starting Xonix Game..." << endl;
    cout.flush();
    
    if (!loadMenuSound()) {
        cout << "Warning: Sound failed to load, continuing anyway..." << endl;
        cout.flush();
        // Don't exit - continue without sound
    }

    srand(time(0));

    cout << "Creating window..." << endl;
    cout.flush();
    
    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix");
    window.setFramerateLimit(60);
    
    cout << "Window created successfully!" << endl;
    cout.flush();
    
    // Initialize global systems
    cout << "Initializing Friend System..." << endl;
    cout.flush();
    g_friendSystem = new FriendSystem();
    
    cout << "Initializing Inventory Manager..." << endl;
    cout.flush();
    g_inventoryMgr = new InventoryManager();
    
    cout << "Systems initialized successfully!" << endl;
    cout.flush();

    // Authentication system
    Authentication auth;
    
    cout << "Authentication system initialized" << endl;
    cout.flush();
    
    // Show login or registration screen
    Font font;
    cout << "Loading font..." << endl;
    cout.flush();
    
    if (!font.loadFromFile("assets/Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        cout.flush();
        return -1;
    }
    
    cout << "Font loaded successfully!" << endl;
    cout.flush();

    // Simple login/register menu
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
                    // Clear events and add minimal delay before showing login
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
                    // Clear events and add minimal delay before showing registration
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
        return 0;
    }

    // Get authenticated player info
    cout << "Getting player info..." << endl;
    cout.flush();
    
    g_currentPlayerID = auth.getCurrentPlayerID();
    g_currentUsername = auth.getCurrentUsername(g_currentPlayerID);
    
    cout << "Logged in as: " << g_currentUsername << " (ID: " << g_currentPlayerID << ")" << endl;
    cout.flush();

    // Add player to friend system
    cout << "Adding player to friend system..." << endl;
    cout.flush();
    
    g_friendSystem->addPlayer(g_currentPlayerID, g_currentUsername);
    
    cout << "Player added successfully!" << endl;
    cout.flush();

    cout << "Checking window status: " << (window.isOpen() ? "OPEN" : "CLOSED") << endl;
    cout.flush();
    
    if (window.isOpen()) {
        cout << "Calling showMenu..." << endl;
        cout.flush();
        showMenu(&window);
    } else {
        cout << "Window was closed during authentication!" << endl;
        cout.flush();
    }

    // Cleanup
    delete g_friendSystem;
    delete g_inventoryMgr;

    return 0;
}
