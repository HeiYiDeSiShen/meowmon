#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib.h>
#include <string>
#include "Catnip.hpp"

class Player {
private:
    std::string name;
    Vector2 position;
    Vector2 velocity;
    float speed;
    bool isMoving;
    
    // 动画相关
    int currentFrame;
    float frameTime;
    float animationSpeed;
    
    // 玩家尺寸
    float width;
    float height;
    
    // 玩家纹理
    Texture2D sprite;
    
    // 灵魂像素动画参数
    float breathTimer;
    float walkTimer;
    bool facingRight;
    
    // 人物纹理路径配置
    std::string texturePath;  // 当前使用的纹理路径
    
    // 猫薄荷
    Catnip catnip;
    
public:
    Player(const std::string& name, Vector2 position);
    ~Player() = default;
    
    // 更新和绘制
    void update(float deltaTime);
    void draw();
    
    // 输入处理
    void handleInput();
    
    // 边界检测
    void checkBoundaries(int mapWidth, int mapHeight);
    
    // 设置和获取
    void setPosition(Vector2 position);
    Vector2 getPosition() const;
    Rectangle getRect() const;
    
    void setSpeed(float speed);
    float getSpeed() const;
    
    void setName(const std::string& name);
    std::string getName() const;
    
    // 纹理管理
    void setTexturePath(const std::string& path);  // 设置新的纹理路径
    void reloadTexture();  // 重新加载纹理
    std::string getTexturePath() const;  // 获取当前纹理路径
    
    // 猫薄荷相关
    void throwCatnip();
    bool hasCatnip() const;
    Vector2 getCatnipPosition() const;
    bool isCatnipActive() const;
    float getCatnipCooldown() const;  // 获取冷却时间
    int getCapturedCount() const;     // 获取抓到数量
    void incrementCapturedCount();    // 增加抓到数量
    
private:
    // 猫薄荷冷却系统
    float catnipCooldownTimer;      // 冷却计时器
    float catnipCooldownDuration;   // 冷却时长
    int capturedCount;              // 抓到的猫咪数量
};

#endif // PLAYER_HPP
