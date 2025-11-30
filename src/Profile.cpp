/*
 * Xonix Game Project
 * Muhammad Amish 24i-2099
 * Saim Zaib 24i-2023
 */

#include "Profile.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cmath>
using namespace std;
using namespace sf;
PlayerProfile::PlayerProfile() {
    playerID = -1;
    username = "";
    totalPoints = 0;
    matchesWon = 0;
    matchesLost = 0;
    currentLevel = 1;
    currentXP = 0;
    xpToNextLevel = 100;
    friendsHead = nullptr;
    matchCount = 0;
    profileFile = "data/profile_0.txt";
}
PlayerProfile::PlayerProfile(int id, string name) {
    playerID = id;
    username = name;
    totalPoints = 0;
    matchesWon = 0;
    matchesLost = 0;
    currentLevel = 1;
    currentXP = 0;
    xpToNextLevel = 100;
    friendsHead = nullptr;
    matchCount = 0;
    profileFile = "data/profile_" + to_string(id) + ".txt";
    loadProfile();
}
PlayerProfile::~PlayerProfile() {
    saveProfile();
    FriendNode* current = friendsHead;
    while (current != nullptr) {
        FriendNode* temp = current;
        current = current->next;
        delete temp;
    }
}
void PlayerProfile::loadProfile() {
    ifstream file(profileFile);
    if (!file.is_open()) {
        cout << "Creating new profile for " << username << endl;
        calculateXPRequirement();
        return;
    }
    file >> playerID >> totalPoints >> matchesWon >> matchesLost >> currentLevel >> currentXP >> matchCount;
    for (int i = 0; i < matchCount && i < 50; i++) {
        file >> matchHistory[i].matchID;
        file.ignore();
        getline(file, matchHistory[i].opponent, ',');
        file >> matchHistory[i].playerScore >> matchHistory[i].opponentScore;
        file >> matchHistory[i].won;
        file.ignore();
        getline(file, matchHistory[i].date);
    }
    int friendCount;
    file >> friendCount;
    for (int i = 0; i < friendCount; i++) {
        int fid;
        string fname;
        file >> fid;
        file.ignore();
        getline(file, fname);
        addFriend(fid, fname);
    }
    file.close();
    calculateXPRequirement();
}
void PlayerProfile::saveProfile() {
    ofstream file(profileFile);
    if (!file.is_open()) {
        cout << "Error: Could not save profile!" << endl;
        return;
    }
    file << playerID << " " << totalPoints << " " << matchesWon << " " << matchesLost << " " 
         << currentLevel << " " << currentXP << " " << matchCount << "\n";
    for (int i = 0; i < matchCount; i++) {
        file << matchHistory[i].matchID << " "
             << matchHistory[i].opponent << ","
             << matchHistory[i].playerScore << " "
             << matchHistory[i].opponentScore << " "
             << matchHistory[i].won << " "
             << matchHistory[i].date << "\n";
    }
    int friendCount = getFriendCount();
    file << friendCount << "\n";
    FriendNode* current = friendsHead;
    while (current != nullptr) {
        file << current->friendPlayerID << " " << current->friendUsername << "\n";
        current = current->next;
    }
    file.close();
}
void PlayerProfile::addPoints(int points) {
    totalPoints += points;
    int xpGained = getXPFromScore(points);
    addXP(xpGained);
    saveProfile();
}
void PlayerProfile::addMatchResult(const string& opponent, int playerScore, int opponentScore, bool won) {
    if (matchCount >= 50) {
        for (int i = 0; i < 49; i++) {
            matchHistory[i] = matchHistory[i + 1];
        }
        matchCount = 49;
    }
    matchHistory[matchCount].matchID = matchCount + 1;
    matchHistory[matchCount].opponent = opponent;
    matchHistory[matchCount].playerScore = playerScore;
    matchHistory[matchCount].opponentScore = opponentScore;
    matchHistory[matchCount].won = won;
    time_t now = time(0);
    char buffer[80];
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
    matchHistory[matchCount].date = string(buffer);
    matchCount++;
    if (won) matchesWon++;
    else matchesLost++;
    saveProfile();
}
void PlayerProfile::addFriend(int friendID, const string& friendName) {
    if (isFriend(friendID)) {
        return;
    }
    FriendNode* newFriend = new FriendNode(friendID, friendName);
    newFriend->next = friendsHead;
    friendsHead = newFriend;
    saveProfile();
}
bool PlayerProfile::removeFriend(int friendID) {
    if (friendsHead == nullptr) return false;
    if (friendsHead->friendPlayerID == friendID) {
        FriendNode* temp = friendsHead;
        friendsHead = friendsHead->next;
        delete temp;
        saveProfile();
        return true;
    }
    FriendNode* current = friendsHead;
    while (current->next != nullptr) {
        if (current->next->friendPlayerID == friendID) {
            FriendNode* temp = current->next;
            current->next = current->next->next;
            delete temp;
            saveProfile();
            return true;
        }
        current = current->next;
    }
    return false;
}
bool PlayerProfile::isFriend(int friendID) {
    FriendNode* current = friendsHead;
    while (current != nullptr) {
        if (current->friendPlayerID == friendID) {
            return true;
        }
        current = current->next;
    }
    return false;
}
int PlayerProfile::getFriendCount() {
    int count = 0;
    FriendNode* current = friendsHead;
    while (current != nullptr) {
        count++;
        current = current->next;
    }
    return count;
}
void PlayerProfile::displayProfile(RenderWindow* window, Font& font) {
    Text title("PLAYER PROFILE", font, 30);
    title.setFillColor(Color::Yellow);
    title.setPosition(200, 30);
    window->draw(title);
    Text info("", font, 20);
    info.setFillColor(Color::White);
    info.setPosition(100, 100);
    string profileText = "Username: " + username + "\n";
    profileText += "Player ID: " + to_string(playerID) + "\n\n";
    profileText += "Level: " + to_string(currentLevel) + " (" + getLevelTitle() + ")\n";
    profileText += "XP: " + to_string(currentXP) + " / " + to_string(xpToNextLevel) + "\n";
    profileText += "Progress: " + to_string(static_cast<int>(getXPProgress() * 100)) + "%\n\n";
    profileText += "Total Points: " + to_string(totalPoints) + "\n";
    profileText += "Matches Won: " + to_string(matchesWon) + "\n";
    profileText += "Matches Lost: " + to_string(matchesLost) + "\n";
    profileText += "Friends: " + to_string(getFriendCount()) + "\n";
    profileText += "\nPress ESC to return";
    info.setString(profileText);
    window->draw(info);
    RectangleShape progressBg(Vector2f(400, 25));
    progressBg.setPosition(100, 250);
    progressBg.setFillColor(Color(50, 50, 50));
    progressBg.setOutlineColor(Color::White);
    progressBg.setOutlineThickness(2);
    window->draw(progressBg);
    RectangleShape progressBar(Vector2f(400 * getXPProgress(), 25));
    progressBar.setPosition(100, 250);
    if (getXPProgress() < 0.33f) {
        progressBar.setFillColor(Color::Red);
    } else if (getXPProgress() < 0.66f) {
        progressBar.setFillColor(Color::Yellow);
    } else {
        progressBar.setFillColor(Color::Green);
    }
    window->draw(progressBar);
}
void PlayerProfile::displayMatchHistory(RenderWindow* window, Font& font) {
    Text title("MATCH HISTORY", font, 28);
    title.setFillColor(Color::Yellow);
    title.setPosition(220, 20);
    window->draw(title);
    int yPos = 70;
    for (int i = matchCount - 1; i >= 0 && i >= matchCount - 8; i--) {
        Text matchText("", font, 16);
        matchText.setPosition(50, yPos);
        string result = matchHistory[i].won ? "WIN" : "LOSS";
        Color color = matchHistory[i].won ? Color::Green : Color::Red;
        string line = result + " vs " + matchHistory[i].opponent + " (" +
                     to_string(matchHistory[i].playerScore) + "-" +
                     to_string(matchHistory[i].opponentScore) + ") " +
                     matchHistory[i].date;
        matchText.setString(line);
        matchText.setFillColor(color);
        window->draw(matchText);
        yPos += 30;
    }
    Text instruction("Press ESC to return", font, 16);
    instruction.setPosition(230, 400);
    instruction.setFillColor(Color(150, 150, 150));
    window->draw(instruction);
}
void PlayerProfile::displayFriendsList(RenderWindow* window, Font& font) {
    Text title("FRIENDS LIST", font, 28);
    title.setFillColor(Color::Yellow);
    title.setPosition(240, 20);
    window->draw(title);
    int yPos = 80;
    int count = 1;
    FriendNode* current = friendsHead;
    while (current != nullptr && count <= 10) {
        Text friendText("", font, 18);
        friendText.setPosition(100, yPos);
        friendText.setString(to_string(count) + ". " + current->friendUsername + " (ID: " + to_string(current->friendPlayerID) + ")");
        friendText.setFillColor(Color::White);
        window->draw(friendText);
        yPos += 30;
        count++;
        current = current->next;
    }
    if (friendsHead == nullptr) {
        Text noFriends("No friends yet. Add some!", font, 18);
        noFriends.setPosition(150, 150);
        noFriends.setFillColor(Color(150, 150, 150));
        window->draw(noFriends);
    }
    Text instruction("Press ESC to return", font, 16);
    instruction.setPosition(230, 400);
    instruction.setFillColor(Color(150, 150, 150));
    window->draw(instruction);
}
void PlayerProfile::calculateXPRequirement() {
    xpToNextLevel = static_cast<int>(100 * pow(currentLevel, 1.5));
}
void PlayerProfile::checkLevelUp() {
    bool leveledUp = false;
    while (currentXP >= xpToNextLevel) {
        currentXP -= xpToNextLevel;
        currentLevel++;
        calculateXPRequirement();
        leveledUp = true;
        cout << "🎉 LEVEL UP! You are now Level " << currentLevel << " - " << getLevelTitle() << "!" << endl;
    }
    if (leveledUp) {
        saveProfile();
    }
}
void PlayerProfile::addXP(int xp) {
    currentXP += xp;
    checkLevelUp();
}
int PlayerProfile::getXPFromScore(int score) const {
    int baseXP = score * 5;
    if (score >= 200) baseXP += 100;       // Epic game bonus
    else if (score >= 150) baseXP += 50;   // Great game bonus
    else if (score >= 100) baseXP += 25;   // Good game bonus
    return baseXP;
}
string PlayerProfile::getLevelTitle() const {
    if (currentLevel >= 50) return "Xonix Legend";
    else if (currentLevel >= 40) return "Grandmaster";
    else if (currentLevel >= 35) return "Master";
    else if (currentLevel >= 30) return "Expert";
    else if (currentLevel >= 25) return "Veteran";
    else if (currentLevel >= 20) return "Professional";
    else if (currentLevel >= 15) return "Advanced";
    else if (currentLevel >= 10) return "Skilled";
    else if (currentLevel >= 5) return "Apprentice";
    else return "Novice";
}
