#ifndef GAME_HPP
#define GAME_HPP

#include <raylib.h>
#include <string>

// 游戏状态枚举
enum GameState {
    GAME_TITLE,
    GAME_PLAYING,
    GAME_BATTLE,
    GAME_MENU,
    GAME_GAME_OVER
};

class Game {
public:
    // 构造函数和析构函数
    Game(int width, int height, const std::string& title);
    ~Game();
    
    // 游戏主循环
    void run();
    
    // 设置当前游戏状态
    void setState(GameState state);
    
    // 获取当前游戏状态
    GameState getState() const;
    
    // 窗口尺寸
    int getWidth() const;
    int getHeight() const;
    
private:
    // 初始化游戏
    void init();
    
    // 更新游戏逻辑
    void update(float deltaTime);
    
    // 绘制游戏画面
    void draw();
    
    // 处理输入
    void handleInput();
    
    // 清理游戏资源
    void cleanup();
    
    // 游戏状态处理函数
    void handleTitleState();
    void handlePlayingState(float deltaTime);
    void handleBattleState(float deltaTime);
    void handleMenuState();
    void handleGameOverState();
    
    // 游戏属性
    int screenWidth;
    int screenHeight;
    std::string windowTitle;
    GameState currentState;
    bool isRunning;
    
    // 性能统计
    float frameTime;
    int fps;
};

#endif // GAME_HPP
