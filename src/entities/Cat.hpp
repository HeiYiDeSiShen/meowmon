#ifndef CAT_HPP
#define CAT_HPP

#include "../core/StatusIndicator.hpp"
#include <raylib.h>
#include <string>
#include <memory>
#include <random>

enum class CatType {
    PERSIAN,
    SIAMESE,
    MAINE_COON,
    RAGDOLL,
    BENGAL
};

enum class CatState {
    NORMAL,
    FLEEING,
    CATNIPPED,
    CAUGHT
};

enum class CatPersonality {
    NORMAL,
    COWARD,     // 胆小：逃跑半径大，速度快
    GREEDY,     // 贪吃：猫薄荷吸引半径大，持续时间长
    CURIOUS     // 好奇：偶尔会主动靠近静止的玩家
};

class Cat {
private:
    std::string name;
    Vector2 position;
    Vector2 velocity;
    float speed;
    CatType type;
    CatState state;
    bool isCaught;
    float catnipTimer;
    float stateTimer;
    Color color;
    
    // 基础属性
    float width, height;
    float legLength;     // 腿长
    float bodyFatness;   // 胖瘦程度 (1.0 为标准)
    bool isFluffy;       // 是否蓬松
    bool isMoving;
    bool facingRight;    // 转向状态
    float breathTimer;  // 呼吸动画计时器
    float walkTimer;    // 走路动画计时器
    float blinkTimer;   // 眨眼计时器
    float blinkDuration; // 眨眼持续时间
    bool isBlinking;    // 是否正在眨眼
    float tailWagTimer; // 尾巴摆动计时器
    int currentFrame;
    float frameTime;
    float animationSpeed;
    Texture2D sprite;
    std::string texturePath;
    
    // 随机数生成
    std::unique_ptr<std::random_device> rd;
    std::unique_ptr<std::mt19937> gen;
    
    // AI相关
    float aiChangeDirectionTimer;
    float aiChangeDirectionInterval;
    int smartMovePattern;
    
    // 猫薄荷效果
    float catnipEffectTimer;
    float baseEffectTime;
    Vector2 catnipPosition;
    
    // 随机印象属性 (Random Impression Traits)
    float earAngle;       // 耳朵角度 (0为立耳, 1.5为飞机耳)
    float eyeSize;        // 眼睛大小缩放
    float whiskerLength;  // 胡须长度
    float tailStyle;      // 尾巴风格 (长度或摆动频率)

    // 稀有度与性格
    bool isShiny;           // 是否是闪光猫
    Color shinyEffectColor; // 闪光特效颜色
    CatPersonality personality;
    float personalityTimer; // 性格行为计时器

    // 状态指示器
    std::unique_ptr<StatusIndicator> statusIndicator;

public:
    // 构造函数和析构函数
    Cat(const std::string& name, Vector2 position, CatType type);
    Cat(const std::string& name, float x, float y, CatType type);
    ~Cat();
    
    // 移动构造函数和赋值运算符
    Cat(Cat&& other) noexcept;
    Cat& operator=(Cat&& other) noexcept;
    
    // 删除拷贝构造函数和拷贝赋值运算符
    Cat(const Cat&) = delete;
    Cat& operator=(const Cat&) = delete;
    
    // 更新和绘制
    void update(float deltaTime, Vector2 playerPosition, Vector2 catnipPosition, bool hasCatnip, int capturedCount);
    void draw();
    void updateTimers(float deltaTime);
    
    // 状态管理
    void setState(CatState newState);
    CatState getState() const { return state; }
    void updateState(Vector2 playerPosition, Vector2 catnipPosition, bool hasCatnip, int capturedCount, float deltaTime);
    
    // 移动相关
    void moveRandomly(float deltaTime);
    void fleeFrom(Vector2 target, float deltaTime);
    void moveTowards(Vector2 target, float deltaTime);
    void updateMovement(float deltaTime);
    void updateAI(float deltaTime, Vector2 playerPos);
    
    // 猫薄荷效果
    void applyCatnipEffect(float duration);
    bool isUnderCatnipEffect() const;
    float getCatnipTimeRemaining() const;
    
    // 获取信息
    std::string getName() const { return name; }
    Vector2 getPosition() const { return position; }
    Vector2 getVelocity() const { return velocity; }
    float getSpeed() const { return speed; }
    CatType getType() const { return type; }
    bool isCaughtStatus() const { return isCaught; }
    bool getIsShiny() const { return isShiny; }
    CatPersonality getPersonality() const { return personality; }
    Color getCatColor() const { return color; }
    Rectangle getRect() const;
    
    // 设置信息
    void setPosition(Vector2 pos);
    void setPosition(float x, float y);
    void setCaught(bool caught);
    void setSpeed(float speed);
    void setName(const std::string& name);
    void setTexturePath(const std::string& path);
    void reloadTexture();
    
    // 类型相关
    std::string getCatTypeName() const;
    float getCatnipSensitivity() const;
    float getBaseSpeed() const;
    void setCatType(CatType type);
    CatType getCatType() const;
    
    // 边界检测
    void checkBoundaries(int mapWidth, int mapHeight);
    bool checkCollision(const Rectangle& playerRect);
    
    // 状态指示器
    void updateStatusIndicator(float deltaTime);
    void drawStatusIndicator();
    
    // 内部AI方法
    void normalAI(float deltaTime, Vector2 playerPos);
    void fleeAI(float deltaTime);
    void catnipAI(float deltaTime);
    void changeRandomDirection();
    bool shouldChangeDirection() const;
    Vector2 generateRandomDirection() const;
    
    // 工具方法
    std::string getTexturePath() const;
    float getBaseEffectTime() const { return baseEffectTime; }
    void setBaseEffectTime(float time) { baseEffectTime = time; }
};

#endif // CAT_HPP
