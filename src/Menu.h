/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
const int Max_menu = 8;
const int pauseMenu = 5;
class MinHeapLeaderboard;
class FriendSystem;
class InventoryManager;
void showMenu(sf::RenderWindow* window); 
void showOptions(sf::RenderWindow* window);  // Function to display options
void showEndMenu(sf::RenderWindow* window, int score);
void showMEndMenu(sf::RenderWindow* window, int score, string string);
void showPauseMenu(sf::RenderWindow* window) ;
void showGameMode(sf::RenderWindow* window); // Function to display game mode options
void showDifficulty(sf::RenderWindow* window); // Function to display difficulty options
void showSound(sf::RenderWindow* window); // Function to display sound options
void showResetSettings(sf::RenderWindow* window); // Function to reset settings
void showScoreBoard(sf::RenderWindow* window);  // Old scoreboard
void showNewLeaderboard(sf::RenderWindow* window, MinHeapLeaderboard* leaderboard);  // New leaderboard
void showLevelSelection(sf::RenderWindow* window);  // New level selection
void showFriendsMenu(sf::RenderWindow* window, FriendSystem* friendSystem, int playerID);  // Friends system
void showInventoryMenu(sf::RenderWindow* window, InventoryManager* inventoryMgr, int playerID, int playerLevel);  // Inventory system 
void showProfileMenu(sf::RenderWindow* window, int playerID, const string& username);  // Profile viewing system
bool selectFriendForMultiplayer(sf::RenderWindow* window, FriendSystem* friendSystem, int playerID, int& selectedPlayerID, string& selectedUsername);  // Friend selector
void showLoadGameMenu(sf::RenderWindow* window);  // Load saved games
void drawMenu(sf::RenderWindow& window, sf::Font& font, sf::Text menu[Max_menu], sf::Color menuColor = sf::Color(169, 169, 169));
void moveUp(int& menuSelected, int Max_menu);
void moveDown(int& menuSelected, int Max_menu);
bool loadMenuSound();
void startBackgroundMusic();
void stopBackgroundMusic();
void drawCommonUI(sf::RenderWindow* window, sf::Sprite& background, sf::Sprite& logo);
