/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Header file declaring menu system functions and constants
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>


const int Max_menu = 6;
const int pauseMenu = 4;



// Forward declarations
class MinHeapLeaderboard;
class FriendSystem;
class InventoryManager;

// Function declarations for the main menu and other related features
void showMenu(sf::RenderWindow* window); 
void showOptions(sf::RenderWindow* window);  // Function to display options
void showEndMenu(sf::RenderWindow* window, int score);
void showMEndMenu(sf::RenderWindow* window, int score, std::string string);
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
bool selectFriendForMultiplayer(sf::RenderWindow* window, FriendSystem* friendSystem, int playerID, int& selectedPlayerID, std::string& selectedUsername);  // Friend selector 


//helper function 

void drawMenu(sf::RenderWindow& window, sf::Font& font, sf::Text menu[Max_menu], sf::Color menuColor = sf::Color(169, 169, 169));
void moveUp(int& menuSelected, int Max_menu);
void moveDown(int& menuSelected, int Max_menu);
bool loadMenuSound();
void drawCommonUI(sf::RenderWindow* window, sf::Sprite& background, sf::Sprite& logo);

