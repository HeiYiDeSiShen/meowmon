#ifndef STARTSCREEN_HPP
#define STARTSCREEN_HPP

#include <raylib.h>
#include "UIHelper.hpp"
#include <string>
#include <vector>

class StartScreen {
private:
    float titleY;
    float subtitleY;
    float buttonY;
    float timer;
    
    // Tailwind Style State
    float entranceAnim;      // 0.0 to 1.0
    float hoverProgress;     // 0.0 to 1.0
    float arrowOffset;       // For the -> animation
    Vector2 mouseGlowPos;    // Following mouse
    
    struct Particle {
        Vector2 position;
        Vector2 velocity;
        float radius;
        float alpha;
        Color color;
    };
    std::vector<Particle> particles;
    
public:
    StartScreen();
    
    // 更新开始界面
    void update(float deltaTime);
    
    // 绘制开始界面
    void draw();
    
    // 检查是否开始游戏
    bool shouldStartGame();
    
private:
    // 绘制标题动画
    void drawTitle();
    
    // 绘制开始按钮
    void drawStartButton();
    
    // 绘制背景
    void drawBackground();
};

#endif // STARTSCREEN_HPP