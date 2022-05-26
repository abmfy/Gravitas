#ifndef SaveManager_h
#define SaveManager_h

extern const int levelCount;

// Singleton
class SaveManager {
    static constexpr const char *filename {"save.dat"};

    static SaveManager instance;

    int level {1};

    SaveManager();
    SaveManager(const SaveManager&) = delete;
    SaveManager(SaveManager&&) = delete;
    SaveManager &operator = (const SaveManager&) = delete;
    SaveManager &operator = (SaveManager&&) = delete;
    ~SaveManager();

public:
    static SaveManager &getInstance();
    
    int getLevel() const;
    void updateLevel(int);
};
#endif