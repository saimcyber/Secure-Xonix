#pragma once
#include <string>
#include <SFML/Graphics.hpp>
struct LeaderboardEntry {
    int playerID;
    std::string username;
    int totalPoints;
    LeaderboardEntry() : playerID(0), username(""), totalPoints(0) {}
    LeaderboardEntry(int id, std::string name, int points) : playerID(id), username(name), totalPoints(points) {}
};
class MinHeapLeaderboard {
private:
    LeaderboardEntry heap[10];  // Fixed size array for top 10
    int size;
    std::string leaderboardFile;
    void heapifyUp(int index);
    void heapifyDown(int index);
    int parent(int index) { return (index - 1) / 2; }
    int leftChild(int index) { return 2 * index + 1; }
    int rightChild(int index) { return 2 * index + 2; }
    void swap(int i, int j);
    void loadLeaderboard();
    void saveLeaderboard();
public:
    MinHeapLeaderboard();
    ~MinHeapLeaderboard();
    void insertOrUpdate(int playerID, const std::string& username, int points);
    LeaderboardEntry getMin();
    bool isFull() { return size == 10; }
    int getSize() { return size; }
    void displayLeaderboard(sf::RenderWindow* window, sf::Font& font);
    LeaderboardEntry* getSortedLeaderboard();  // Returns array sorted descending
};
