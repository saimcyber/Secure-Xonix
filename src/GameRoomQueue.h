/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
struct GameRoomNode {
    int playerID;
    string username;
    int totalPoints;
    GameRoomNode* next;
    GameRoomNode(int id, string name, int points) 
        : playerID(id), username(name), totalPoints(points), next(nullptr) {}
};
class GameRoomQueue {
private:
    GameRoomNode* front;  // First in queue
    GameRoomNode* rear;   // Last in queue
    int size;
public:
    GameRoomQueue();
    ~GameRoomQueue();
    void enqueue(int playerID, const string& username, int totalPoints);
    GameRoomNode* dequeue();  // Remove and return front player
    GameRoomNode* peek();     // View front without removing
    bool isEmpty() { return front == nullptr; }
    int getSize() { return size; }
    bool canMatch() { return size >= 2; }  // Need 2 players to start game
    void displayQueue();
};
