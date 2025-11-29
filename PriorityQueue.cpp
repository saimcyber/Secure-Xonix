/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Priority Queue matchmaking implementation
 */

#include "PriorityQueue.h"
#include <iostream>

using namespace std;

// Constructor
MatchmakingPriorityQueue::MatchmakingPriorityQueue() {
    size = 0;
}

// Swap two elements in heap
void MatchmakingPriorityQueue::swap(int i, int j) {
    MatchmakingPlayer temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

// Heapify up (for insertion) - Max heap, so higher priority goes up
void MatchmakingPriorityQueue::heapifyUp(int index) {
    if (index == 0) return;
    
    int parentIdx = parent(index);
    // Max heap: parent should be >= child
    if (heap[index].priority > heap[parentIdx].priority) {
        swap(index, parentIdx);
        heapifyUp(parentIdx);
    }
}

// Heapify down (for deletion) - Max heap
void MatchmakingPriorityQueue::heapifyDown(int index) {
    int left = leftChild(index);
    int right = rightChild(index);
    int largest = index;
    
    // Find largest among node and its children
    if (left < size && heap[left].priority > heap[largest].priority) {
        largest = left;
    }
    if (right < size && heap[right].priority > heap[largest].priority) {
        largest = right;
    }
    
    // If largest is not current node, swap and continue
    if (largest != index) {
        swap(index, largest);
        heapifyDown(largest);
    }
}

// Enqueue a player (insert into heap)
void MatchmakingPriorityQueue::enqueue(int playerID, const string& username, int totalPoints) {
    if (size >= 100) {
        cout << "Matchmaking queue is full!" << endl;
        return;
    }
    
    // Add new player at end
    heap[size] = MatchmakingPlayer(playerID, username, totalPoints);
    heapifyUp(size);
    size++;
    
    cout << "Player " << username << " (Points: " << totalPoints << ") added to matchmaking queue." << endl;
}

// Dequeue highest priority player (remove from heap)
MatchmakingPlayer MatchmakingPriorityQueue::dequeue() {
    if (isEmpty()) {
        cout << "Queue is empty!" << endl;
        return MatchmakingPlayer();
    }
    
    // Root has highest priority
    MatchmakingPlayer highest = heap[0];
    
    // Move last element to root and heapify down
    heap[0] = heap[size - 1];
    size--;
    
    if (size > 0) {
        heapifyDown(0);
    }
    
    return highest;
}

// Peek at highest priority player without removing
MatchmakingPlayer MatchmakingPriorityQueue::peek() {
    if (isEmpty()) {
        cout << "Queue is empty!" << endl;
        return MatchmakingPlayer();
    }
    return heap[0];
}
