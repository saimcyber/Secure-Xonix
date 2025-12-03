/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#pragma once
#include <string>
using namespace std;
struct MatchmakingPlayer {
    int playerID;
    string username;
    int totalPoints;
    int priority;  // Higher points = higher priority
    MatchmakingPlayer() : playerID(0), username(""), totalPoints(0), priority(0) {}
    MatchmakingPlayer(int id, string name, int points) 
        : playerID(id), username(name), totalPoints(points), priority(points) {}
};
class MatchmakingPriorityQueue {
private:
    MatchmakingPlayer heap[100];  // Max 100 players in queue
    int size;
    void heapifyUp(int index);
    void heapifyDown(int index);
    int parent(int index) { return (index - 1) / 2; }
    int leftChild(int index) { return 2 * index + 1; }
    int rightChild(int index) { return 2 * index + 2; }
    void swap(int i, int j);
public:
    MatchmakingPriorityQueue();
    void enqueue(int playerID, const string& username, int totalPoints);
    MatchmakingPlayer dequeue();  // Remove and return highest priority player
    MatchmakingPlayer peek();     // View highest priority without removing
    bool isEmpty() { return size == 0; }
    int getSize() { return size; }
    bool canMatch() { return size >= 2; }  // Need at least 2 players to match
};
