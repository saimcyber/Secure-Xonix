/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Priority Queue for matchmaking based on player total points
 *              Higher points = higher priority for matching
 */

#pragma once

#include <string>

// Player node for matchmaking queue
struct MatchmakingPlayer {
    int playerID;
    std::string username;
    int totalPoints;
    int priority;  // Higher points = higher priority
    
    MatchmakingPlayer() : playerID(0), username(""), totalPoints(0), priority(0) {}
    MatchmakingPlayer(int id, std::string name, int points) 
        : playerID(id), username(name), totalPoints(points), priority(points) {}
};

// Priority Queue using max-heap (highest priority at top)
class MatchmakingPriorityQueue {
private:
    MatchmakingPlayer heap[100];  // Max 100 players in queue
    int size;
    
    // Heap operations
    void heapifyUp(int index);
    void heapifyDown(int index);
    int parent(int index) { return (index - 1) / 2; }
    int leftChild(int index) { return 2 * index + 1; }
    int rightChild(int index) { return 2 * index + 2; }
    void swap(int i, int j);

public:
    MatchmakingPriorityQueue();
    
    // Queue operations
    void enqueue(int playerID, const std::string& username, int totalPoints);
    MatchmakingPlayer dequeue();  // Remove and return highest priority player
    MatchmakingPlayer peek();     // View highest priority without removing
    bool isEmpty() { return size == 0; }
    int getSize() { return size; }
    
    // Matchmaking helper
    bool canMatch() { return size >= 2; }  // Need at least 2 players to match
};
