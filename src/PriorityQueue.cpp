/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#include "PriorityQueue.h"
#include <iostream>
using namespace std;
MatchmakingPriorityQueue::MatchmakingPriorityQueue() {
    size = 0;
}
void MatchmakingPriorityQueue::swap(int i, int j) {
    MatchmakingPlayer temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}
void MatchmakingPriorityQueue::heapifyUp(int index) {
    if (index == 0) return;
    int parentIdx = parent(index);
    if (heap[index].priority > heap[parentIdx].priority) {
        swap(index, parentIdx);
        heapifyUp(parentIdx);
    }
}
void MatchmakingPriorityQueue::heapifyDown(int index) {
    int left = leftChild(index);
    int right = rightChild(index);
    int largest = index;
    if (left < size && heap[left].priority > heap[largest].priority) {
        largest = left;
    }
    if (right < size && heap[right].priority > heap[largest].priority) {
        largest = right;
    }
    if (largest != index) {
        swap(index, largest);
        heapifyDown(largest);
    }
}
void MatchmakingPriorityQueue::enqueue(int playerID, const string& username, int totalPoints) {
    if (size >= 100) {
        cout << "Matchmaking queue is full!" << endl;
        return;
    }
    heap[size] = MatchmakingPlayer(playerID, username, totalPoints);
    heapifyUp(size);
    size++;
    cout << "Player " << username << " (Points: " << totalPoints << ") added to matchmaking queue." << endl;
}
MatchmakingPlayer MatchmakingPriorityQueue::dequeue() {
    if (isEmpty()) {
        cout << "Queue is empty!" << endl;
        return MatchmakingPlayer();
    }
    MatchmakingPlayer highest = heap[0];
    heap[0] = heap[size - 1];
    size--;
    if (size > 0) {
        heapifyDown(0);
    }
    return highest;
}
MatchmakingPlayer MatchmakingPriorityQueue::peek() {
    if (isEmpty()) {
        cout << "Queue is empty!" << endl;
        return MatchmakingPlayer();
    }
    return heap[0];
}
