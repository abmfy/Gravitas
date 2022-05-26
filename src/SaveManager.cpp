#include <fstream>
using std::ifstream, std::ofstream;

#include "SaveManager.h"

SaveManager SaveManager::instance;

SaveManager::SaveManager() {
    if (int level; ifstream {filename} >> level) {
        updateLevel(level);
    }
}

SaveManager::~SaveManager() {
    ofstream fout {filename};
    fout << level;
}

SaveManager &SaveManager::getInstance() {
    return instance;
}

int SaveManager::getLevel() const {
    return level;
}

void SaveManager::updateLevel(int newLevel) {
    if (level < newLevel && newLevel <= levelCount) {
        level = newLevel;
    }
}