/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Game Room Queue using linked list - FIFO for waiting players
 */

#pragma once

#include <string>

// Node for game room queue
struct GameRoomNode {
    int playerID;
    std::string username;
    int totalPoints;
    GameRoomNode* next;
    
    GameRoomNode(int id, std::string name, int points) 
        : playerID(id), username(name), totalPoints(points), next(nullptr) {}
};

// Game Room Queue (FIFO using linked list)
class GameRoomQueue {
private:
    GameRoomNode* front;  // First in queue
    GameRoomNode* rear;   // Last in queue
    int size;

public:
    GameRoomQueue();
    ~GameRoomQueue();
    
    // Queue operations
    void enqueue(int playerID, const std::string& username, int totalPoints);
    GameRoomNode* dequeue();  // Remove and return front player
    GameRoomNode* peek();     // View front without removing
    bool isEmpty() { return front == nullptr; }
    int getSize() { return size; }
    
    // Matchmaking helper
    bool canMatch() { return size >= 2; }  // Need 2 players to start game
    
    // Display all waiting players
    void displayQueue();
};
