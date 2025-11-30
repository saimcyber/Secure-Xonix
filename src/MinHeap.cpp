#include "MinHeap.h"
#include <fstream>
#include <iostream>
using namespace std;
using namespace sf;
MinHeapLeaderboard::MinHeapLeaderboard() {
    size = 0;
    leaderboardFile = "data/leaderboard.txt";
    loadLeaderboard();
}
MinHeapLeaderboard::~MinHeapLeaderboard() {
    saveLeaderboard();
}
void MinHeapLeaderboard::swap(int i, int j) {
    LeaderboardEntry temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}
void MinHeapLeaderboard::heapifyUp(int index) {
    if (index == 0) return;
    int parentIndex = parent(index);
    if (heap[index].totalPoints < heap[parentIndex].totalPoints) {
        swap(index, parentIndex);
        heapifyUp(parentIndex);
    }
}
void MinHeapLeaderboard::heapifyDown(int index) {
    int smallest = index;
    int left = leftChild(index);
    int right = rightChild(index);
    if (left < size && heap[left].totalPoints < heap[smallest].totalPoints) {
        smallest = left;
    }
    if (right < size && heap[right].totalPoints < heap[smallest].totalPoints) {
        smallest = right;
    }
    if (smallest != index) {
        swap(index, smallest);
        heapifyDown(smallest);
    }
}
LeaderboardEntry MinHeapLeaderboard::getMin() {
    if (size == 0) {
        return LeaderboardEntry();
    }
    return heap[0];
}
void MinHeapLeaderboard::insertOrUpdate(int playerID, const string& username, int points) {
    int existingIndex = -1;
    for (int i = 0; i < size; i++) {
        if (heap[i].playerID == playerID || heap[i].username == username) {
            existingIndex = i;
            break;
        }
    }
    if (existingIndex != -1) {
        int oldPoints = heap[existingIndex].totalPoints;
        heap[existingIndex].totalPoints = points;
        heap[existingIndex].playerID = playerID;  // Update ID in case it changed
        heap[existingIndex].username = username;  // Update username in case it changed
        if (points < oldPoints) {
            heapifyUp(existingIndex);
        } else {
            heapifyDown(existingIndex);
        }
        saveLeaderboard();
        return;
    }
    if (size < 10) {
        heap[size] = LeaderboardEntry(playerID, username, points);
        heapifyUp(size);
        size++;
        saveLeaderboard();
        return;
    }
    if (points > heap[0].totalPoints) {
        heap[0] = LeaderboardEntry(playerID, username, points);
        heapifyDown(0);
        saveLeaderboard();
    }
}
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
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapifyDown(i);
    }
    cout << "Loaded " << size << " entries into leaderboard." << endl;
}
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
LeaderboardEntry* MinHeapLeaderboard::getSortedLeaderboard() {
    static LeaderboardEntry sorted[10];
    for (int i = 0; i < size; i++) {
        sorted[i] = heap[i];
    }
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
