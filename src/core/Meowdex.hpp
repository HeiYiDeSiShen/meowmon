#ifndef MEOWDEX_HPP
#define MEOWDEX_HPP

#include "../entities/Cat.hpp"
#include <string>
#include <vector>
#include <map>

struct MeowdexEntry {
    CatType type;
    std::string speciesName;
    int caughtCount = 0;
    bool discoveredShiny = false;
    std::vector<CatPersonality> discoveredPersonalities;
    std::string description;
    
    // 互动系统数据
    float affection = 0.0f;     // 好感度 (0-100)
    int feedCount = 0;          // 喂食次数
    int playCount = 0;          // 玩耍次数
    float lastInteractionTime = 0.0f;
};

class Meowdex {
private:
    std::map<CatType, MeowdexEntry> entries;
    bool isVisible;
    
    // 详情模式状态
    bool isDetailMode;
    CatType selectedType;
    float detailAnimationTimer;
    
    // 3D 模式支持
    Camera3D camera;
    float rotationAngle;
    bool is3DMode;

    // 交互反馈
    float feedbackTimer;
    std::string feedbackMessage;
    float catBounceY;

public:
    Meowdex();
    void recordCapture(const Cat& cat);
    void update(float deltaTime);
    void draw();
    void toggleVisibility();
    bool getIsVisible() const { return isVisible; }
    void setVisible(bool visible);
    
    // 互动指令
    void interact(const std::string& action);

    // 数据持久化
    void saveProgress();
    void loadProgress();

private:
    void initEntries();
    void drawListView(Font font, bool hasFont);
    void drawDetailView(Font font, bool hasFont);
};

#endif // MEOWDEX_HPP
