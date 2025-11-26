/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: [Student Name 1], [Student Name 2]
 * Roll Numbers: [Roll# 1], [Roll# 2]
 * Date: November 2025
 * Description: Min-Heap based leaderboard for top 10 players
 */

#pragma once

#include <string>
#include <SFML/Graphics.hpp>

// Leaderboard entry
struct LeaderboardEntry {
    int playerID;
    std::string username;
    int totalPoints;
    
    LeaderboardEntry() : playerID(0), username(""), totalPoints(0) {}
    LeaderboardEntry(int id, std::string name, int points) : playerID(id), username(name), totalPoints(points) {}
};

// Min-Heap for Top 10 players
class MinHeapLeaderboard {
private:
    LeaderboardEntry heap[10];  // Fixed size array for top 10
    int size;
    std::string leaderboardFile;
    
    // Heap operations
    void heapifyUp(int index);
    void heapifyDown(int index);
    int parent(int index) { return (index - 1) / 2; }
    int leftChild(int index) { return 2 * index + 1; }
    int rightChild(int index) { return 2 * index + 2; }
    void swap(int i, int j);
    
    // File operations
    void loadLeaderboard();
    void saveLeaderboard();

public:
    MinHeapLeaderboard();
    ~MinHeapLeaderboard();
    
    // Main operations
    void insertOrUpdate(int playerID, const std::string& username, int points);
    LeaderboardEntry getMin();
    bool isFull() { return size == 10; }
    int getSize() { return size; }
    
    // Display
    void displayLeaderboard(sf::RenderWindow* window, sf::Font& font);
    LeaderboardEntry* getSortedLeaderboard();  // Returns array sorted descending
};
