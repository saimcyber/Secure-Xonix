/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Complete menu system including main menu, options, pause menu,
 *              end game menu, scoreboard display, and all UI navigation
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Menu.h"
#include "main.h"
#include "MinHeap.h"
#include "FriendSystem.h"
#include "Inventory.h"
#include "Profile.h"
#include <time.h>
#include <string>

using namespace sf;
using namespace std;




// ------------------ GLOBAL VARIABLES ------------------

int sound = 1;
int mode = 1; // Global game mode: 1=single player, 2=multiplayer
SoundBuffer changeBuffer;
Sound changeSound;
extern int difficulty;  // Defined in Menu.cpp showDifficulty function

// Import global player info
extern int g_currentPlayerID;
extern std::string g_currentUsername;



// ------------------ HELPER FUNCTIONS ------------------


   

bool loadMenuSound() {
    if (!changeBuffer.loadFromFile("audio/change.wav")) {
        cout << "Failed to load sound\n";
        return false;
    }
    changeSound.setBuffer(changeBuffer);
    return true;
}


void drawMenu(RenderWindow& window, Font& font, Text menu[Max_menu], Color menuColor) {
    for (int i = 0; i < Max_menu; i++) {
        
        menu[i].setFillColor(menuColor);

        window.draw(menu[i]);
    }
}

void moveUp(int& menuSelected, int Max_menu) {

    if (sound == 1) changeSound.play();  //  Only play if sound is ON

    (menuSelected == 0)? menuSelected = Max_menu - 1: menuSelected--; 
   
    
}

void moveDown(int& menuSelected, int Max_menu) {
    
    
    if (sound == 1) changeSound.play();  //  Only play if sound is ON

   
    (menuSelected == Max_menu - 1)? menuSelected = 0 :menuSelected++;

    
   
}

void drawCommonUI(RenderWindow* window, Sprite& background, Sprite& logo) {
    if (background.getTexture())
        window->draw(background);
    if (logo.getTexture())
        window->draw(logo);
}

// ------------------ SHOW SCORE BOARD ------------------

void showScoreBoard(RenderWindow* window) {
    // Load font
    Font font;
    if (!font.loadFromFile("Fonts/OpenSans.ttf")) {
        cout << "Font not loaded!" << endl;
        return;
    }

    

    // Read scores from file
    string lines[5];
    ifstream infile("scores.txt");
    int i = 0;
    while (i < 5 && getline(infile, lines[i])) {
        if (!lines[i].empty()) {  // Only count non-empty lines
            i++;
        }
    }
    infile.close();

    // Clear screen
    window->clear(Color::Black);

    Texture backgroundTexture;
    Sprite background;
     if (backgroundTexture.loadFromFile("images/Black.jpeg"))
        background.setTexture(backgroundTexture);


    Texture titleTexture;
    Sprite title;
    if (titleTexture.loadFromFile("images/SCORE-TITLE.png")) {
        title.setTexture(titleTexture);
        title.setPosition(150, 10);
        title.setScale(350.0f / title.getLocalBounds().width, 150.0f / title.getLocalBounds().height);

        
    }
    drawCommonUI(window, background, title);


    Text test("(Press  Ecs to exit)", font, 20);
    test.setFillColor(Color(255, 255, 100)); // Light yellow
    test.setPosition(200, 400);
    window->draw(test);


    
   
    // Display each score on score board
    for (int j = 0; j < i; j++) {
        Text scoreText(lines[j], font, 28);
        scoreText.setFillColor(Color(220, 220, 220)); // Light gray for better visibility
        scoreText.setPosition(200, 120 + j * 40); // space between scores
        window->draw(scoreText);
    }
    
    window->display();

    // Wait for key press to exit scoreboard
    while (true) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::KeyPressed || event.type == Event::Closed  ) {
                return;
            }
        }
    }
}

// ------------------ NEW LEADERBOARD (MIN-HEAP) ------------------
void showNewLeaderboard(RenderWindow* window, MinHeapLeaderboard* leaderboard) {
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return;
    }

    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/Black.jpeg"))
        background.setTexture(backgroundTexture);

    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::KeyPressed || event.type == Event::Closed) {
                return;
            }
        }

        window->clear();
        window->draw(background);
        leaderboard->displayLeaderboard(window, font);
        window->display();
    }
}


// ------------------  GENERIC SUB-MENU RENDERER ------------------
int showSubmenu(RenderWindow* window, const string options[], int count) {
    int selected = 0;

    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return -1;
    }
    
    // Get equipped theme colors - will be refreshed in loop
    extern InventoryManager* g_inventoryMgr;
    extern int g_currentPlayerID;
    Theme* equippedTheme = nullptr;
    Color themeColor = Color::Blue;
    Color themeSecondary = Color::Cyan;

    // Load background image
    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    // Load logo image
    Texture logoTexture;
    Sprite logo;
    if (logoTexture.loadFromFile("images/logo.png")) {
        logo.setTexture(logoTexture);
        logo.setPosition(30, 120);
        logo.setScale(350.0f / logo.getLocalBounds().width, 90.0f / logo.getLocalBounds().height);
    }

    // Setup text for each option
    Text optionText[10];  // Max 10 menu items
    for (int i = 0; i < count; i++) {
        optionText[i].setFont(font);
        optionText[i].setCharacterSize(20);
        optionText[i].setFillColor(Color(169, 169, 169));  // Default gray
        optionText[i].setString(options[i]);
        optionText[i].setPosition(60, 230 + i * 30);  // Position options vertically
    }

    // Selector highlight box
    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));  // Semi-transparent gray

    // Menu loop
    while (window->isOpen()) {
        // Refresh theme colors every frame
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();

            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up)
                    moveUp(selected, count);
                else if (event.key.code == Keyboard::Down)
                    moveDown(selected, count);
                else if (event.key.code == Keyboard::Return)
                    return selected;  // Return selected index to caller function
            }
        }

        // Drawing the UI
        window->clear(themeColor);
        drawCommonUI(window, background, logo);

        // Draw selector on selected option
        selector.setPosition(55, 230 + selected * 30);
        window->draw(selector);

        // Draw menu options with hover effect
        for (int i = 0; i < count; i++) {
            optionText[i].setFillColor(i == selected ? themeSecondary : Color(200, 200, 200));
            window->draw(optionText[i]);
        }

        window->display();
    }

    return -1;  // If window is closed or ESC is pressed
}

// ------------------ LEVEL SELECTION ------------------
void showLevelSelection(RenderWindow* window) {
    string options[] = { "Level 1 - Tutorial", "Level 2 - Easy", "Level 3 - Medium", "Level 4 - Hard", "Level 5 - Expert", "Back" };
    int choice = showSubmenu(window, options, 6);
    
    if (choice >= 0 && choice < 5) {
        // Level selected - store in difficulty global
        difficulty = choice + 1;
        cout << "Selected Level: " << choice + 1 << endl;
    }
}


// ------------------ SUB-MENU ------------------

void showGameMode(RenderWindow* window) {
    string options[] = { "Single Player", "Multiplayer", "Back" };
    int choice = showSubmenu(window, options, 3);
      
    if (choice == 0) {
        mode = 1;
        showMenu(window);  // Re-show menu after returning
    }
    else if (choice == 1) {
        mode = 2;
        showMenu(window);  // Re-show menu after returning
    }
    // If choice == 2, user selected "Back", so simply return to caller
}
    
      int difficulty = 1; // default difficulty is easy
void showDifficulty(RenderWindow* window) {
    string options[] = { "Easy", "Medium", "Hard", "Continous", "Back" };

    int choice = showSubmenu(window, options, 5);
       
    if (choice == 0) {
        difficulty = 1;
        showMenu(window);  // Re-show menu after returning
    }
    else if (choice == 1) {
        difficulty = 2;
        showMenu(window);  // Re-show menu after returning
    }
    else if (choice == 2) {
        difficulty = 3;
        showMenu(window);  // Re-show menu after returning
    }
    else if (choice == 3) {
        difficulty = 4;
        showMenu(window);  // Re-show menu after returning
    }
    // If choice == 4, user selected "Back", so simply return to caller
}

   
void showSound(RenderWindow* window) {
    string options[] = { "Sound Off", "Sound On", "Back" };
    int choice = showSubmenu(window, options, 3);

    
    if (choice == 0) {
        sound = 0; // 
        showMenu(window);  //  return to main menu after returning
    }
    else if (choice == 1) {
        sound = 1;
        showMenu(window);  // Return to main menu after returning
    }
    // If choice == 2, user selected "Back", so simply return to caller
}


     int reset   = 0; // default mode sound on 
void showResetSettings(RenderWindow* window) {
        string options[] = { "Confirm", "Cancel" };
        int choice =  showSubmenu(window, options, 2);
        if (choice == 0) {
        reset = 1; // 
        showMenu(window);  //  return to main menu after returning
    }
     // If choice == 1, user selected "Cancel", so simply return to caller
}





// ------------------ OPTION MENU ------------------

void showOptions(RenderWindow* window) {
    int selected = 0;
    string options[] = { "GAME MODE", "DIFFICULTY","SCORE BOARD" ,"SOUND", "RESET SETTINGS", "BACK" };
    int count = 6;

    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return;
    }
    
    // Get equipped theme colors - will be refreshed in loop
    extern InventoryManager* g_inventoryMgr;
    extern int g_currentPlayerID;
    Theme* equippedTheme = nullptr;
    Color themeColor = Color::Blue;
    Color themeSecondary = Color::Cyan;

    // Background & Logo
    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    Texture logoTexture;
    Sprite logo;
    if (logoTexture.loadFromFile("images/logo.png")) {
        logo.setTexture(logoTexture);
        logo.setPosition(30, 120);
        logo.setScale(350.0f / logo.getLocalBounds().width, 90.0f / logo.getLocalBounds().height);
        
    }

    Text texts[10];
    for (int i = 0; i < count; ++i) {
        texts[i].setFont(font);
        texts[i].setCharacterSize(20);
        texts[i].setFillColor(Color(169, 169, 169));
        texts[i].setString(options[i]);
        texts[i].setPosition(60, 230 + i * 30);
    }

    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));

    while (window->isOpen()) {
        // Refresh theme colors every frame
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up  )
                 moveUp(selected, count);
                else if (event.key.code == Keyboard::Down )
                moveDown(selected, count);
                else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) showGameMode(window);
                    else if (selected == 1) showDifficulty(window);
                    else if (selected == 2) {
                        MinHeapLeaderboard leaderboard;
                        showNewLeaderboard(window, &leaderboard);
                    }
                    else if (selected == 3) showSound(window);
                    else if (selected == 4) showResetSettings(window);
                    else if (selected == 5) return;
                }
            }
        }

        window->clear(themeColor);
        drawCommonUI(window, background, logo);

        selector.setPosition(55, 230 + selected * 30);
        window->draw(selector);

        for (int i = 0; i < count; i++) {
            texts[i].setFillColor(i == selected ? themeSecondary : Color(200, 200, 200));
            window->draw(texts[i]);
        }

        window->display();
    }
}




// ------------------ MAIN MENU ------------------

void showMenu(RenderWindow* window) {
    int selected = 0;  // index selected

    cout << "Entering main menu..." << endl;
    cout.flush();
    
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed in showMenu!" << endl;
        cout.flush();
        return;
    }
    
    cout << "Font loaded in menu!" << endl;
    cout.flush();
    
    // Clear any pending events to prevent auto-triggering
    Event clearEvent;
    while (window->pollEvent(clearEvent)) { }
    sf::sleep(sf::milliseconds(100));  // Reduced delay
    while (window->pollEvent(clearEvent)) { }

    Text menu[Max_menu];
    string items[Max_menu] = { "PLAY", "OPTIONS", "FRIENDS", "INVENTORY", "ABOUT", "EXIT" };
    for (int i = 0; i < Max_menu; ++i) {
        menu[i].setFont(font);
        menu[i].setCharacterSize(20);
        // Set default text color to gray using RGB (169, 169, 169)
        menu[i].setFillColor(Color(169, 169, 169));
        menu[i].setString(items[i]);
        menu[i].setPosition(60, 230 + i * 30);
    }

    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));
    selector.setPosition(55, 230);

    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    Texture logoTexture;
    Sprite logo;
    if (logoTexture.loadFromFile("images/logo.png")) {
        logo.setTexture(logoTexture);
        logo.setPosition(30, 120);
        logo.setScale(350.0f / logo.getLocalBounds().width, 90.0f / logo.getLocalBounds().height);
    }

    // Load player profile to display level and XP
    PlayerProfile currentProfile(g_currentPlayerID, g_currentUsername);
    
    // Get equipped theme colors - will be refreshed in the loop
    extern InventoryManager* g_inventoryMgr;
    Theme* equippedTheme = nullptr;
    Color themeColor = Color::Blue;
    Color themeSecondary = Color::Cyan;
    
    // Create text for player info
    Text playerInfoText;
    playerInfoText.setFont(font);
    playerInfoText.setCharacterSize(20);
    playerInfoText.setStyle(Text::Bold);
    playerInfoText.setPosition(430, 15); // Top right corner

    while (window->isOpen()) {
        // Refresh theme colors every frame to reflect changes from inventory
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        playerInfoText.setFillColor(themeSecondary); // Update player info color
        
        Event event;
        while (window->pollEvent(event)) { // event listner 
            if (event.type == Event::Closed)
                window->close();
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up)
                    moveUp(selected, Max_menu);
                else if (event.key.code == Keyboard::Down)
                    moveDown(selected, Max_menu);
                else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) {
                        // Directly start game with current mode and difficulty
                        if (mode == 1) {
                            SingleGame(window, difficulty, g_currentPlayerID, g_currentUsername);
                            // Reload profile after game ends to show updated stats
                            currentProfile = PlayerProfile(g_currentPlayerID, g_currentUsername);
                        } else if (mode == 2) {
                            // Multiplayer - select second player from friends
                            extern FriendSystem* g_friendSystem;
                            int player2ID = 0;
                            string player2Name = "";
                            
                            if (selectFriendForMultiplayer(window, g_friendSystem, g_currentPlayerID, player2ID, player2Name)) {
                                MultiGame(window, difficulty, g_currentPlayerID, g_currentUsername, player2ID, player2Name);
                                // Reload profile after game ends to show updated stats
                                currentProfile = PlayerProfile(g_currentPlayerID, g_currentUsername);
                            }
                            // If selection cancelled, just return to menu
                        }
                    } else if (selected == 1) {
                        showOptions(window);
                    } else if (selected == 2) {
                        // Friends menu
                        extern FriendSystem* g_friendSystem;
                        showFriendsMenu(window, g_friendSystem, g_currentPlayerID);
                    } else if (selected == 3) {
                        // Inventory menu
                        extern InventoryManager* g_inventoryMgr;
                        PlayerProfile tempProfile(g_currentPlayerID, g_currentUsername);
                        showInventoryMenu(window, g_inventoryMgr, g_currentPlayerID, tempProfile.getCurrentLevel());
                        // Reload profile after inventory (in case auto-unlocks affected level)
                        currentProfile = PlayerProfile(g_currentPlayerID, g_currentUsername);
                    } else if (selected == 4) {
                        cout<<"Made with love by Saim Zaib and Amish Munir"<<endl;
                    } else if (selected == 5) {
                        window->close();
                    }
                }
            }
        }

        window->clear(themeColor);
        drawCommonUI(window, background, logo);

        selector.setPosition(55, 230 + selected * 30);
        window->draw(selector);

        // Update text colors for hover effect (selected option uses secondary color)
        for (int i = 0; i < Max_menu; i++) {
            if (i == selected) {
                menu[i].setFillColor(themeSecondary);  // Hover color (theme secondary)
            } else {
                menu[i].setFillColor(Color(200, 200, 200));   // Default color (light gray)
            }
            window->draw(menu[i]);
        }

        // Display player info
        string playerInfo = "PLAYER INFO\n";
        playerInfo += "-------------\n";
        playerInfo += g_currentUsername + "\n";
        playerInfo += "Lvl " + to_string(currentProfile.getCurrentLevel()) + " - " + currentProfile.getLevelTitle() + "\n";
        playerInfo += "XP: " + to_string(currentProfile.getCurrentXP()) + "/" + to_string(currentProfile.getXPToNextLevel()) + "\n";
        
        // Progress bar
        int barLength = 15;
        int filled = static_cast<int>(currentProfile.getXPProgress() * barLength);
        string progressBar = "[";
        for (int i = 0; i < barLength; i++) {
            progressBar += (i < filled) ? "=" : "-";
        }
        progressBar += "] " + to_string(static_cast<int>(currentProfile.getXPProgress() * 100)) + "%";
        playerInfo += progressBar;
        
        playerInfoText.setString(playerInfo);
        window->draw(playerInfoText);

        window->display();
    }
}



// ------------------ PAUSE MENU ------------------

void showPauseMenu(RenderWindow* window) {
    int selected = 0;

    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return;
    }
    
    // Get equipped theme colors - will be refreshed in loop
    extern InventoryManager* g_inventoryMgr;
    extern int g_currentPlayerID;
    Theme* equippedTheme = nullptr;
    Color themeColor = Color::Blue;
    Color themeSecondary = Color::Cyan;

    Text menu[pauseMenu];
    string items[pauseMenu] = { "RESUME", "RESTART", "MAIN MENU", "EXIT" };
    for (int i = 0; i < pauseMenu; i++) {
        menu[i].setFont(font);
        menu[i].setCharacterSize(20);
        // Set default text color to gray using RGB (169, 169, 169)
        menu[i].setFillColor(Color(169, 169, 169));
        menu[i].setString(items[i]);
        menu[i].setPosition(60, 230 + i * 30);
    }

    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));
    selector.setPosition(55, 230);

    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    Texture logoTexture;
    Sprite logo;
    if (logoTexture.loadFromFile("images/logo.png")) {
        logo.setTexture(logoTexture);
        logo.setPosition(30, 120);
        logo.setScale(350.0f / logo.getLocalBounds().width, 80.0f / logo.getLocalBounds().height);
    }

    while (window->isOpen()) {
        // Refresh theme colors every frame
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up)
                    moveUp(selected, pauseMenu);
                else if (event.key.code == Keyboard::Down)
                     moveDown(selected, pauseMenu);
                else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) {
                        return; // return to contine game 
                    } else if (selected == 1) {
                       if (mode == 1){
                            SingleGame(window, difficulty, g_currentPlayerID, g_currentUsername);
                          }
                             else if(mode == 2) {
                                   // TODO: Implement proper second player for multiplayer restart
                                   MultiGame(window, difficulty, g_currentPlayerID, g_currentUsername, g_currentPlayerID, g_currentUsername);
                                   }
                    } else if (selected == 2) {
                       showMenu(window);
                    } else if (selected == 3) {
                        window->close();
                    }
                }
            }
        }

        window->clear(themeColor);
        drawCommonUI(window, background, logo);

        selector.setPosition(55, 230 + selected * 30);
        window->draw(selector);

        // Update text colors for hover effect (selected option uses theme secondary)
        for (int i = 0; i < pauseMenu; i++) {
            if (i == selected) {
                menu[i].setFillColor(themeSecondary);  // Hover color (theme secondary)
            } else {
                menu[i].setFillColor(Color(200, 200, 200));   // Default color (light gray)
            }
            window->draw(menu[i]);
        }

        window->display();
    }
} 


// ------------------ END MENU  ------------------
void showEndMenu(RenderWindow* window, int score) {
    int selected = 0;

    // Load font
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return;
    }
    
    // Get equipped theme colors
    extern InventoryManager* g_inventoryMgr;
    extern int g_currentPlayerID;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;

    // Load background
    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    // Load game over image
    Texture gameOverTexture;
    Sprite gameOver;
    if (gameOverTexture.loadFromFile("images/gameover.png")) {
        gameOver.setTexture(gameOverTexture);
        gameOver.setPosition(120, 50);
    }

    // Score text
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(35);
    scoreText.setString("Your Score: " + to_string(score));
    scoreText.setFillColor(themeSecondary); // Use theme secondary color
    scoreText.setPosition(250, 190);

    // High score text
    Text highScoreText;
    highScoreText.setFont(font);
    highScoreText.setCharacterSize(24);
    highScoreText.setFillColor(Color(100, 255, 100)); // Bright green
    highScoreText.setPosition(60, 220);

    // Check if score is in top 5
    int scores[5];
    int count = 0;
    ifstream infile("scores.txt");
    while (infile >> scores[count] && count < 5) {
        count++;
    }
    infile.close();

    bool isHighScore = false;
    if (count < 5 || score > scores[count - 1]) {
        isHighScore = true;
        highScoreText.setString("New High Score!");
    }

    // Menu options
    const int options = 3;
    string items[options] = { "RESTART", "MAIN MENU", "EXIT" };
    Text menu[options];

    for (int i = 0; i < options; i++) {
        menu[i].setFont(font);
        menu[i].setCharacterSize(20);
        menu[i].setFillColor(Color(169, 169, 169));
        menu[i].setString(items[i]);
        menu[i].setPosition(60, 270 + i * 30);
    }

    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));
    selector.setPosition(55, 270);

    while (window->isOpen()) {
        // Refresh theme colors every frame
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        scoreText.setFillColor(themeSecondary); // Update score color
        
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up)
                    moveUp(selected, options);
                else if (event.key.code == Keyboard::Down)
                    moveDown(selected, options);
                else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) {
                        // Restart in single player mode only - multiplayer needs proper implementation
                        SingleGame(window, difficulty, g_currentPlayerID, g_currentUsername);
                    } else if (selected == 1) {
                        showMenu(window);
                    } else if (selected == 2) {
                        window->close();
                    }
                }
            }
        }

        window->clear(themeColor);
        drawCommonUI(window, background, gameOver);

        window->draw(scoreText);
        if (isHighScore)
            window->draw(highScoreText);

        selector.setPosition(55, 270 + selected * 30);
        window->draw(selector);

        for (int i = 0; i < options; i++) {
            if (i == selected)
                menu[i].setFillColor(themeSecondary);
            else
                menu[i].setFillColor(Color(200, 200, 200));
            window->draw(menu[i]);
        }

        window->display();
    }
}

void showMEndMenu(RenderWindow* window, int score,  string string) {
    int selected = 0;

    // Load font
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Font loading failed!" << endl;
        return;
    }
    
    // Get equipped theme colors
    extern InventoryManager* g_inventoryMgr;
    extern int g_currentPlayerID;
    Theme* equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
    Color themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
    Color themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;

    // Load background
    Texture backgroundTexture;
    Sprite background;
    if (backgroundTexture.loadFromFile("images/background.jpg"))
        background.setTexture(backgroundTexture);

    // Load game over image
    Texture gameOverTexture;
    Sprite gameOver;
    if (gameOverTexture.loadFromFile("images/gameover.png")) {
        gameOver.setTexture(gameOverTexture);
        gameOver.setPosition(120, 50);
    }

    // Score text
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(35);
    scoreText.setString(string + " Score: " + to_string(score));
    scoreText.setFillColor(themeSecondary); // Use theme secondary color
    scoreText.setPosition(250, 190);

    // High score text
    Text winner;
    winner.setFont(font);
    winner.setCharacterSize(24);
    winner.setFillColor(Color(100, 255, 100)); // Bright green
    winner.setPosition(60, 220);
    winner.setString(string);
    

   

    // Menu options
    const int options = 3;
    std::string items[options] = { "RESTART", "MAIN MENU", "EXIT" };
    Text menu[options];

    for (int i = 0; i < options; i++) {
        menu[i].setFont(font);
        menu[i].setCharacterSize(20);
        menu[i].setFillColor(Color(169, 169, 169));
        menu[i].setString(items[i]);
        menu[i].setPosition(60, 270 + i * 30);
    }

    RectangleShape selector(Vector2f(200, 25));
    selector.setFillColor(Color(128, 128, 128, 150));
    selector.setPosition(55, 270);

    while (window->isOpen()) {
        // Refresh theme colors every frame
        equippedTheme = g_inventoryMgr->getEquippedTheme(g_currentPlayerID);
        themeColor = equippedTheme ? equippedTheme->primaryColor : Color::Blue;
        themeSecondary = equippedTheme ? equippedTheme->secondaryColor : Color::Cyan;
        scoreText.setFillColor(themeSecondary); // Update score color
        
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up)
                    moveUp(selected, options);
                else if (event.key.code == Keyboard::Down)
                    moveDown(selected, options);
                else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) {
                       if(mode == 1) SingleGame(window, difficulty, g_currentPlayerID, g_currentUsername);
                       else if(mode == 2) MultiGame(window, difficulty, g_currentPlayerID, g_currentUsername, g_currentPlayerID, g_currentUsername);
                    } else if (selected == 1) {
                        showMenu(window);
                    } else if (selected == 2) {
                        window->close();
                    }
                }
            }
        }

        window->clear(themeColor);
        drawCommonUI(window, background, gameOver);

        window->draw(scoreText);
       
        

        selector.setPosition(55, 270 + selected * 30);
        window->draw(selector);

        for (int i = 0; i < options; i++) {
            if (i == selected)
                menu[i].setFillColor(themeSecondary);
            else
                menu[i].setFillColor(Color(200, 200, 200));
            window->draw(menu[i]);
        }

        window->display();
    }
}
// ===== Friends Menu Implementation =====
void showFriendsMenu(sf::RenderWindow* window, FriendSystem* friendSystem, int playerID) {
    using namespace sf;
    using namespace std;
    
    // Clear any pending events to prevent auto-triggering
    Event clearEvent;
    while (window->pollEvent(clearEvent)) { }
    sf::sleep(sf::milliseconds(100));
    while (window->pollEvent(clearEvent)) { }
    
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Error loading font" << endl;
        return;
    }
    
    Texture bgTexture;
    if (!bgTexture.loadFromFile("images/background.jpg")) {
        cout << "Error loading background" << endl;
        return;
    }
    Sprite background(bgTexture);
    
    Text title("FRIENDS MENU", font, 40);
    title.setPosition(250, 50);
    title.setFillColor(Color::Yellow);
    
    const int menuItems = 5;
    Text menu[menuItems];
    string menuText[menuItems] = {
        "View Friends List",
        "View Pending Requests",
        "Send Friend Request",
        "Manage Requests",
        "Back to Menu"
    };
    
    for (int i = 0; i < menuItems; i++) {
        menu[i].setFont(font);
        menu[i].setString(menuText[i]);
        menu[i].setCharacterSize(24);
        menu[i].setPosition(250, 150 + i * 50);
    }
    
    int selected = 0;
    RectangleShape selector(Vector2f(400, 40));
    selector.setFillColor(Color(128, 128, 128, 150));
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up) {
                    selected = (selected - 1 + menuItems) % menuItems;
                } else if (event.key.code == Keyboard::Down) {
                    selected = (selected + 1) % menuItems;
                } else if (event.key.code == Keyboard::Return) {
                    if (selected == 0) {
                        // View friends list - console output only
                        friendSystem->viewFriendsList(playerID);
                    } else if (selected == 1) {
                        // View pending requests - console output only
                        friendSystem->viewPendingRequests(playerID);
                    } else if (selected == 2) {
                        // Send friend request - GUI text input
                        string friendUsername = "";
                        bool inputActive = true;
                        
                        while (inputActive && window->isOpen()) {
                            Event inputEvent;
                            while (window->pollEvent(inputEvent)) {
                                if (inputEvent.type == Event::Closed) {
                                    window->close();
                                    return;
                                }
                                
                                if (inputEvent.type == Event::TextEntered) {
                                    if (inputEvent.text.unicode == '\b' && friendUsername.length() > 0) {
                                        // Backspace
                                        friendUsername.pop_back();
                                    } else if (inputEvent.text.unicode == '\r' || inputEvent.text.unicode == '\n') {
                                        // Enter - send request
                                        if (!friendUsername.empty()) {
                                            friendSystem->sendFriendRequest(playerID, friendUsername);
                                        }
                                        inputActive = false;
                                    } else if (inputEvent.text.unicode == 27) {
                                        // Escape - cancel
                                        inputActive = false;
                                    } else if (inputEvent.text.unicode < 128 && inputEvent.text.unicode >= 32) {
                                        // Regular character
                                        friendUsername += static_cast<char>(inputEvent.text.unicode);
                                    }
                                }
                            }
                            
                            // Draw input screen
                            window->clear();
                            window->draw(background);
                            
                            Text inputTitle("Enter Friend's Username", font, 30);
                            inputTitle.setPosition(200, 200);
                            inputTitle.setFillColor(Color::Yellow);
                            window->draw(inputTitle);
                            
                            RectangleShape inputBox(Vector2f(400, 50));
                            inputBox.setPosition(200, 270);
                            inputBox.setFillColor(Color(50, 50, 50));
                            inputBox.setOutlineColor(Color::White);
                            inputBox.setOutlineThickness(2);
                            window->draw(inputBox);
                            
                            Text inputText(friendUsername, font, 24);
                            inputText.setPosition(210, 280);
                            inputText.setFillColor(Color::White);
                            window->draw(inputText);
                            
                            Text instruction("Press ENTER to send or ESC to cancel", font, 18);
                            instruction.setPosition(200, 350);
                            instruction.setFillColor(Color(200, 200, 200));
                            window->draw(instruction);
                            
                            window->display();
                        }
                    } else if (selected == 3) {
                        // Accept/Reject requests - GUI version
                        bool manageActive = true;
                        int requestSelected = 0;
                        
                        while (manageActive && window->isOpen()) {
                            // Get pending requests count
                            int pendingCount = friendSystem->getPendingRequestsCount(playerID);
                            
                            if (pendingCount == 0) {
                                // No pending requests
                                window->clear();
                                window->draw(background);
                                
                                Text noRequests("No pending requests!", font, 30);
                                noRequests.setPosition(250, 250);
                                noRequests.setFillColor(Color::Yellow);
                                window->draw(noRequests);
                                
                                Text pressBack("Press ESC to go back", font, 20);
                                pressBack.setPosition(270, 350);
                                pressBack.setFillColor(Color::White);
                                window->draw(pressBack);
                                
                                window->display();
                                
                                Event noReqEvent;
                                while (window->pollEvent(noReqEvent)) {
                                    if (noReqEvent.type == Event::Closed) {
                                        window->close();
                                        return;
                                    }
                                    if (noReqEvent.type == Event::KeyReleased && noReqEvent.key.code == Keyboard::Escape) {
                                        manageActive = false;
                                    }
                                }
                                continue;
                            }
                            
                            Event manageEvent;
                            while (window->pollEvent(manageEvent)) {
                                if (manageEvent.type == Event::Closed) {
                                    window->close();
                                    return;
                                }
                                
                                if (manageEvent.type == Event::KeyReleased) {
                                    if (manageEvent.key.code == Keyboard::Up) {
                                        requestSelected = (requestSelected - 1 + pendingCount) % pendingCount;
                                    } else if (manageEvent.key.code == Keyboard::Down) {
                                        requestSelected = (requestSelected + 1) % pendingCount;
                                    } else if (manageEvent.key.code == Keyboard::A) {
                                        // Accept request
                                        FriendRequest* current = friendSystem->getPendingRequestsHead(playerID);
                                        for (int i = 0; i < requestSelected && current; i++) {
                                            current = current->next;
                                        }
                                        if (current) {
                                            friendSystem->acceptFriendRequest(playerID, current->fromPlayerID);
                                            requestSelected = 0;
                                        }
                                    } else if (manageEvent.key.code == Keyboard::R) {
                                        // Reject request
                                        FriendRequest* current = friendSystem->getPendingRequestsHead(playerID);
                                        for (int i = 0; i < requestSelected && current; i++) {
                                            current = current->next;
                                        }
                                        if (current) {
                                            friendSystem->rejectFriendRequest(playerID, current->fromPlayerID);
                                            requestSelected = 0;
                                        }
                                    } else if (manageEvent.key.code == Keyboard::Escape) {
                                        manageActive = false;
                                    }
                                }
                            }
                            
                            // Draw manage requests screen
                            window->clear();
                            window->draw(background);
                            
                            Text manageTitle("Manage Friend Requests", font, 30);
                            manageTitle.setPosition(200, 50);
                            manageTitle.setFillColor(Color::Yellow);
                            window->draw(manageTitle);
                            
                            Text instructions("UP/DOWN: Navigate | A: Accept | R: Reject | ESC: Back", font, 16);
                            instructions.setPosition(120, 100);
                            instructions.setFillColor(Color(200, 200, 200));
                            window->draw(instructions);
                            
                            // Display pending requests
                            FriendRequest* current = friendSystem->getPendingRequestsHead(playerID);
                            int idx = 0;
                            int yPos = 150;
                            
                            while (current) {
                                RectangleShape requestBox(Vector2f(600, 40));
                                requestBox.setPosition(100, yPos);
                                
                                if (idx == requestSelected) {
                                    requestBox.setFillColor(Color(100, 100, 100));
                                } else {
                                    requestBox.setFillColor(Color(50, 50, 50));
                                }
                                requestBox.setOutlineColor(Color::White);
                                requestBox.setOutlineThickness(1);
                                window->draw(requestBox);
                                
                                Text requestText("From: " + current->fromUsername, font, 20);
                                requestText.setPosition(110, yPos + 10);
                                requestText.setFillColor(Color::White);
                                window->draw(requestText);
                                
                                current = current->next;
                                idx++;
                                yPos += 50;
                            }
                            
                            window->display();
                        }
                    } else if (selected == 4) {
                        return; // Back to menu
                    }
                }
            }
        }
        
        window->clear();
        window->draw(background);
        window->draw(title);
        
        selector.setPosition(245, 145 + selected * 50);
        window->draw(selector);
        
        for (int i = 0; i < menuItems; i++) {
            if (i == selected)
                menu[i].setFillColor(Color::White);
            else
                menu[i].setFillColor(Color(200, 200, 200));
            window->draw(menu[i]);
        }
        
        window->display();
    }
}

// ===== Inventory Menu Implementation =====
void showInventoryMenu(sf::RenderWindow* window, InventoryManager* inventoryMgr, int playerID, int playerLevel) {
    using namespace sf;
    using namespace std;
    
    // Auto-unlock themes based on current level
    inventoryMgr->autoUnlockByLevel(playerID, playerLevel);
    
    // Clear any pending events to prevent auto-triggering
    Event clearEvent;
    while (window->pollEvent(clearEvent)) { }
    sf::sleep(sf::milliseconds(100));
    while (window->pollEvent(clearEvent)) { }
    
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Error loading font" << endl;
        return;
    }
    
    Texture bgTexture;
    if (!bgTexture.loadFromFile("images/background.jpg")) {
        cout << "Error loading background" << endl;
        return;
    }
    Sprite background(bgTexture);
    
    Text title("THEME INVENTORY", font, 36);
    title.setPosition(220, 20);
    title.setFillColor(Color(0, 255, 255));
    title.setStyle(Text::Bold);
    
    Text levelInfo("Your Level: " + to_string(playerLevel), font, 20);
    levelInfo.setPosition(30, 70);
    levelInfo.setFillColor(Color::Yellow);
    
    int selected = 0;
    const int totalThemes = 10;
    int scrollOffset = 0;
    const int visibleThemes = 6;
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up) {
                    selected = (selected - 1 + totalThemes) % totalThemes;
                    if (selected < scrollOffset) scrollOffset = selected;
                    if (selected >= scrollOffset + visibleThemes) scrollOffset = selected - visibleThemes + 1;
                } else if (event.key.code == Keyboard::Down) {
                    selected = (selected + 1) % totalThemes;
                    if (selected < scrollOffset) scrollOffset = selected;
                    if (selected >= scrollOffset + visibleThemes) scrollOffset = selected - visibleThemes + 1;
                } else if (event.key.code == Keyboard::Return) {
                    int themeID = selected + 1;
                    Theme* theme = inventoryMgr->getThemeTree()->searchTheme(themeID);
                    PlayerInventory* pInv = inventoryMgr->getPlayerInventory(playerID);
                    
                    if (pInv && pInv->unlockedThemes[selected]) {
                        inventoryMgr->equipTheme(playerID, themeID);
                        cout << "Equipped: " << theme->name << endl;
                    } else {
                        cout << "Theme is locked!" << endl;
                    }
                } else if (event.key.code == Keyboard::Escape) {
                    return; // Back to menu
                }
            }
        }
        
        window->clear();
        window->draw(background);
        window->draw(title);
        window->draw(levelInfo);
        
        // Draw theme list
        PlayerInventory* pInv = inventoryMgr->getPlayerInventory(playerID);
        int yPos = 110;
        
        for (int i = scrollOffset; i < scrollOffset + visibleThemes && i < totalThemes; i++) {
            Theme* theme = inventoryMgr->getThemeTree()->searchTheme(i + 1);
            if (!theme) continue;
            
            bool isUnlocked = pInv && pInv->unlockedThemes[i];
            bool isEquipped = pInv && (pInv->equippedThemeID == i + 1);
            bool canUnlock = !theme->isPremium && theme->unlockLevel <= playerLevel;
            
            // Background box
            RectangleShape themeBox(Vector2f(680, 50));
            themeBox.setPosition(20, yPos);
            
            if (i == selected) {
                themeBox.setFillColor(Color(100, 100, 100, 200));
                themeBox.setOutlineColor(Color::Cyan);
                themeBox.setOutlineThickness(2);
            } else {
                themeBox.setFillColor(Color(50, 50, 50, 150));
                themeBox.setOutlineThickness(0);
            }
            window->draw(themeBox);
            
            // Color preview
            RectangleShape colorPreview(Vector2f(40, 40));
            colorPreview.setPosition(25, yPos + 5);
            colorPreview.setFillColor(theme->primaryColor);
            colorPreview.setOutlineColor(theme->secondaryColor);
            colorPreview.setOutlineThickness(3);
            window->draw(colorPreview);
            
            // Theme name
            Text themeName(theme->name, font, 22);
            themeName.setPosition(75, yPos + 5);
            themeName.setFillColor(isUnlocked ? Color::White : Color(150, 150, 150));
            themeName.setStyle(isEquipped ? Text::Bold : Text::Regular);
            window->draw(themeName);
            
            // Status text
            Text status("", font, 18);
            status.setPosition(350, yPos + 10);
            
            if (isEquipped) {
                status.setString("[EQUIPPED]");
                status.setFillColor(Color::Green);
            } else if (isUnlocked) {
                status.setString("[UNLOCKED] - Press Enter");
                status.setFillColor(Color(100, 255, 100));
            } else if (!canUnlock && theme->unlockLevel > playerLevel) {
                status.setString("Level " + to_string(theme->unlockLevel) + " Required");
                status.setFillColor(Color::Red);
            } else if (theme->isPremium) {
                status.setString("[PREMIUM]");
                status.setFillColor(Color::Yellow);
            }
            window->draw(status);
            
            yPos += 55;
        }
        
        // Instructions
        Text instructions("Arrow Keys: Navigate | Enter: Equip | ESC: Back", font, 16);
        instructions.setPosition(180, 420);
        instructions.setFillColor(Color(200, 200, 200));
        window->draw(instructions);
        
        window->display();
    }
}

// ===== Friend Selector for Multiplayer =====
bool selectFriendForMultiplayer(sf::RenderWindow* window, FriendSystem* friendSystem, int playerID, int& selectedPlayerID, std::string& selectedUsername) {
    using namespace sf;
    using namespace std;
    
    // Clear any pending events
    Event clearEvent;
    while (window->pollEvent(clearEvent)) { }
    sf::sleep(sf::milliseconds(100));
    while (window->pollEvent(clearEvent)) { }
    
    Font font;
    if (!font.loadFromFile("Fonts/AlexandriaFLF.ttf")) {
        cout << "Error loading font" << endl;
        return false;
    }
    
    Texture bgTexture;
    if (!bgTexture.loadFromFile("images/background.jpg")) {
        cout << "Error loading background" << endl;
        return false;
    }
    Sprite background(bgTexture);
    
    // Get friends list
    vector<pair<int, string>> friendsList;
    int playerIndex = friendSystem->findPlayerIndex(playerID);
    if (playerIndex == -1) {
        cout << "Player not found!" << endl;
        return false;
    }
    
    // Traverse friends linked list
    FriendSystem::FriendNode* current = friendSystem->players[playerIndex].friendsHead;
    while (current != nullptr) {
        // Friend is either the fromPlayer or toPlayer (depending on who sent the request)
        int friendID = (current->fromPlayerID == playerID) ? current->toPlayerID : current->fromPlayerID;
        std::string friendName = (current->fromPlayerID == playerID) ? current->toUsername : current->fromUsername;
        
        int friendIndex = friendSystem->findPlayerIndex(friendID);
        if (friendIndex != -1) {
            friendsList.push_back(make_pair(friendID, friendName));
        }
        current = current->next;
    }
    
    if (friendsList.empty()) {
        // Show message that no friends available
        Text message("No friends available. Add friends first!", font, 24);
        message.setPosition(150, 200);
        message.setFillColor(Color::Yellow);
        
        Text backMsg("Press ESC to go back", font, 20);
        backMsg.setPosition(250, 250);
        backMsg.setFillColor(Color::White);
        
        while (window->isOpen()) {
            Event event;
            while (window->pollEvent(event)) {
                if (event.type == Event::Closed)
                    window->close();
                if (event.type == Event::KeyReleased && event.key.code == Keyboard::Escape)
                    return false;
            }
            
            window->clear();
            window->draw(background);
            window->draw(message);
            window->draw(backMsg);
            window->display();
        }
        return false;
    }
    
    Text title("SELECT PLAYER 2", font, 36);
    title.setPosition(220, 50);
    title.setFillColor(Color::Yellow);
    
    int selected = 0;
    RectangleShape selector(Vector2f(400, 40));
    selector.setFillColor(Color(128, 128, 128, 150));
    
    vector<Text> friendTexts;
    for (size_t i = 0; i < friendsList.size(); i++) {
        Text friendText(friendsList[i].second, font, 24);
        friendText.setPosition(250, 150 + i * 50);
        friendTexts.push_back(friendText);
    }
    
    Text instructions("Arrow Keys: Navigate | Enter: Select | ESC: Cancel", font, 18);
    instructions.setPosition(150, 400);
    instructions.setFillColor(Color(200, 200, 200));
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed)
                window->close();
            
            if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Up) {
                    selected = (selected - 1 + friendsList.size()) % friendsList.size();
                } else if (event.key.code == Keyboard::Down) {
                    selected = (selected + 1) % friendsList.size();
                } else if (event.key.code == Keyboard::Return) {
                    selectedPlayerID = friendsList[selected].first;
                    selectedUsername = friendsList[selected].second;
                    return true;
                } else if (event.key.code == Keyboard::Escape) {
                    return false;
                }
            }
        }
        
        window->clear();
        window->draw(background);
        window->draw(title);
        window->draw(instructions);
        
        selector.setPosition(245, 145 + selected * 50);
        window->draw(selector);
        
        for (size_t i = 0; i < friendTexts.size(); i++) {
            if (i == selected)
                friendTexts[i].setFillColor(Color::White);
            else
                friendTexts[i].setFillColor(Color(200, 200, 200));
            window->draw(friendTexts[i]);
        }
        
        window->display();
    }
    
    return false;
}


