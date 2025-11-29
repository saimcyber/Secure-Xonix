/*
 * Project: Xonix Game - Data Structures Project
 * Course: Data Structures
 * Authors: M. Amish, Saim Zaib
 * Roll Numbers: 24i-2099, 24i-2023
 * Date: November 2025
 * Description: Game Room Queue implementation
 */

#include "GameRoomQueue.h"
#include <iostream>

using namespace std;

// Constructor
GameRoomQueue::GameRoomQueue()
{
    front = nullptr;
    rear = nullptr;
    size = 0;
}

// Destructor - clean up all nodes
GameRoomQueue::~GameRoomQueue()
{
    while (!isEmpty())
    {
        GameRoomNode *temp = dequeue();
        delete temp;
    }
}

// Enqueue a player at the rear
void GameRoomQueue::enqueue(int playerID, const string &username, int totalPoints)
{
    GameRoomNode *newNode = new GameRoomNode(playerID, username, totalPoints);

    if (isEmpty())
    {
        front = rear = newNode;
    }
    else
    {
        rear->next = newNode;
        rear = newNode;
    }

    size++;
    cout << "Player " << username << " joined game room queue (Position: " << size << ")" << endl;
}

// Dequeue player from front
GameRoomNode *GameRoomQueue::dequeue()
{
    if (isEmpty())
    {
        cout << "Game room queue is empty!" << endl;
        return nullptr;
    }

    GameRoomNode *temp = front;
    front = front->next;

    if (front == nullptr)
    {
        rear = nullptr; // Queue is now empty
    }

    size--;
    return temp;
}

// Peek at front player
GameRoomNode *GameRoomQueue::peek()
{
    if (isEmpty())
    {
        cout << "Game room queue is empty!" << endl;
        return nullptr;
    }
    return front;
}

// Display all players in queue
void GameRoomQueue::displayQueue()
{
    if (isEmpty())
    {
        cout << "No players in game room queue." << endl;
        return;
    }

    cout << "=== Game Room Queue ===" << endl;
    GameRoomNode *current = front;
    int position = 1;

    while (current != nullptr)
    {
        cout << position << ". " << current->username
             << " (Points: " << current->totalPoints << ")" << endl;
        current = current->next;
        position++;
    }
}
