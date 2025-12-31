#ifndef CATNIP_HPP
#define CATNIP_HPP

#include <raylib.h>
#include <string>
#include <vector>

class Catnip {
private:
    Vector2 position;
    Vector2 velocity;
    Vector2 targetPosition;
    bool isThrown;
    bool isActive;
    float lifetime;
    float maxLifetime;
    float effectRadius;
    
    // 物理仿真参数
    float throwTimer;       // 投掷时间计时器
    float totalThrowTime;   // 总投掷预计时间
    float startZ;           // 初始高度
    float currentZ;         // 当前高度（模拟 Z 轴）
    Vector2 startPosition;  // 起始位置
    
public:
    Catnip();
    
    // 抛出猫薄荷
    void throwCatnip(Vector2 startPos, Vector2 targetPos, float power);
    
    // 更新和绘制
    void update(float deltaTime);
    void draw();
    
    // 边界检测
    void checkBoundaries(int mapWidth, int mapHeight);
    
    // 检查是否在效果范围内
    bool isInEffectRange(Vector2 catPosition);
    
    // 获取属性
    Vector2 getPosition() const { return position; }
    bool getIsActive() const { return isActive; }
    float getEffectRadius() const { return effectRadius; }
    
    // 重置
    void reset();
};

#endif // CATNIP_HPP
