/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: [Student Name 1], [Student Name 2]
 * Roll Numbers: [Roll# 1], [Roll# 2]
 * Date: November 2025
 * Description: Min-Heap leaderboard implementation
 */

#include "MinHeap.h"
#include <fstream>
#include <iostream>

using namespace std;
using namespace sf;

// Constructor
MinHeapLeaderboard::MinHeapLeaderboard() {
    size = 0;
    leaderboardFile = "leaderboard.txt";
    loadLeaderboard();
}

// Destructor
MinHeapLeaderboard::~MinHeapLeaderboard() {
    saveLeaderboard();
}

// Swap two elements
void MinHeapLeaderboard::swap(int i, int j) {
    LeaderboardEntry temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

// Heapify up (for insertion)
void MinHeapLeaderboard::heapifyUp(int index) {
    if (index == 0) return;
    
    int parentIndex = parent(index);
    
    // Min-heap property: parent should be smaller
    if (heap[index].totalPoints < heap[parentIndex].totalPoints) {
        swap(index, parentIndex);
        heapifyUp(parentIndex);
    }
}

// Heapify down (for deletion/replacement)
void MinHeapLeaderboard::heapifyDown(int index) {
    int smallest = index;
    int left = leftChild(index);
    int right = rightChild(index);
    
    // Find smallest among node and its children
    if (left < size && heap[left].totalPoints < heap[smallest].totalPoints) {
        smallest = left;
    }
    
    if (right < size && heap[right].totalPoints < heap[smallest].totalPoints) {
        smallest = right;
    }
    
    // If smallest is not the current node, swap and continue
    if (smallest != index) {
        swap(index, smallest);
        heapifyDown(smallest);
    }
}

// Get minimum element (root)
LeaderboardEntry MinHeapLeaderboard::getMin() {
    if (size == 0) {
        return LeaderboardEntry();
    }
    return heap[0];
}

// Insert or update player score
void MinHeapLeaderboard::insertOrUpdate(int playerID, const string& username, int points) {
    // First, check if player already exists in heap
    int existingIndex = -1;
    for (int i = 0; i < size; i++) {
        if (heap[i].playerID == playerID) {
            existingIndex = i;
            break;
        }
    }
    
    // If player exists, update their score
    if (existingIndex != -1) {
        int oldPoints = heap[existingIndex].totalPoints;
        heap[existingIndex].totalPoints = points;
        
        // Restore heap property
        if (points < oldPoints) {
            heapifyUp(existingIndex);
        } else {
            heapifyDown(existingIndex);
        }
        saveLeaderboard();
        return;
    }
    
    // If heap is not full, insert directly
    if (size < 10) {
        heap[size] = LeaderboardEntry(playerID, username, points);
        heapifyUp(size);
        size++;
        saveLeaderboard();
        return;
    }
    
    // If heap is full, check if new score is greater than min (root)
    if (points > heap[0].totalPoints) {
        // Replace root with new entry
        heap[0] = LeaderboardEntry(playerID, username, points);
        heapifyDown(0);
        saveLeaderboard();
    }
}

// Load leaderboard from file
void MinHeapLeaderboard::loadLeaderboard() {
    ifstream file(leaderboardFile);
    if (!file.is_open()) {
        cout << "Creating new leaderboard." << endl;
        return;
    }
    
    size = 0;
    while (size < 10 && !file.eof()) {
        int id, points;
        string name;
        
        file >> id;
        if (file.fail()) break;
        
        file.ignore();
        getline(file, name, ',');
        file >> points;
        
        heap[size] = LeaderboardEntry(id, name, points);
        size++;
    }
    
    file.close();
    
    // Rebuild heap property (in case file was manually edited)
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyDown(i);
    }
    
    cout << "Loaded " << size << " entries into leaderboard." << endl;
}

// Save leaderboard to file
void MinHeapLeaderboard::saveLeaderboard() {
    ofstream file(leaderboardFile);
    if (!file.is_open()) {
        cout << "Error: Could not save leaderboard!" << endl;
        return;
    }
    
    for (int i = 0; i < size; i++) {
        file << heap[i].playerID << " " << heap[i].username << "," << heap[i].totalPoints << "\n";
    }
    
    file.close();
}

// Get sorted leaderboard (descending order)
LeaderboardEntry* MinHeapLeaderboard::getSortedLeaderboard() {
    static LeaderboardEntry sorted[10];
    
    // Copy heap to temporary array
    for (int i = 0; i < size; i++) {
        sorted[i] = heap[i];
    }
    
    // Simple selection sort (descending)
    for (int i = 0; i < size - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < size; j++) {
            if (sorted[j].totalPoints > sorted[maxIdx].totalPoints) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            LeaderboardEntry temp = sorted[i];
            sorted[i] = sorted[maxIdx];
            sorted[maxIdx] = temp;
        }
    }
    
    return sorted;
}

// Display leaderboard
void MinHeapLeaderboard::displayLeaderboard(RenderWindow* window, Font& font) {
    Text title("TOP 10 LEADERBOARD", font, 32);
    title.setFillColor(Color::Yellow);
    title.setPosition(170, 30);
    window->draw(title);
    
    LeaderboardEntry* sorted = getSortedLeaderboard();
    
    int yPos = 90;
    for (int i = 0; i < size; i++) {
        Text rankText("", font, 20);
        rankText.setPosition(80, yPos);
        
        string rankStr = to_string(i + 1) + ". ";
        string line = rankStr + sorted[i].username + " - " + to_string(sorted[i].totalPoints) + " pts";
        
        rankText.setString(line);
        
        // Color code: Gold, Silver, Bronze for top 3
        if (i == 0) rankText.setFillColor(Color(255, 215, 0));      // Gold
        else if (i == 1) rankText.setFillColor(Color(192, 192, 192)); // Silver
        else if (i == 2) rankText.setFillColor(Color(205, 127, 50));  // Bronze
        else rankText.setFillColor(Color::White);
        
        window->draw(rankText);
        yPos += 35;
    }
    
    if (size == 0) {
        Text empty("No entries yet. Play to get on the leaderboard!", font, 18);
        empty.setPosition(100, 150);
        empty.setFillColor(Color(150, 150, 150));
        window->draw(empty);
    }
    
    Text instruction("Press ESC to return", font, 16);
    instruction.setPosition(230, 420);
    instruction.setFillColor(Color(150, 150, 150));
    window->draw(instruction);
}
