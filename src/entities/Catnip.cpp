#include "Catnip.hpp"
#include <cmath>
#include <iostream>

Catnip::Catnip()
    : position({0, 0}), velocity({0, 0}), targetPosition({0, 0}),
      isThrown(false), isActive(false), lifetime(0.0f), maxLifetime(5.0f),
      effectRadius(80.0f), throwTimer(0.0f), totalThrowTime(0.0f), 
      startZ(0.0f), currentZ(0.0f), startPosition({0, 0}) {
}

void Catnip::throwCatnip(Vector2 startPos, Vector2 targetPos, float power) {
    startPosition = startPos;
    position = startPos;
    
    // 计算方向和距离
    Vector2 direction = {targetPos.x - startPos.x, targetPos.y - startPos.y};
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    // 限制最大投掷距离
    float maxDistance = 200.0f; 
    if (distance > maxDistance) {
        float ratio = maxDistance / distance;
        targetPosition.x = startPos.x + direction.x * ratio;
        targetPosition.y = startPos.y + direction.y * ratio;
        distance = maxDistance;
    } else {
        targetPosition = targetPos;
    }
    
    // 设置投掷参数
    float throwSpeed = 400.0f; 
    totalThrowTime = distance / throwSpeed;
    throwTimer = 0.0f;
    currentZ = 0.0f;
    
    isThrown = true;
    isActive = true;
    lifetime = 0.0f;
}

void Catnip::update(float deltaTime) {
    if (!isActive) return;
    
    lifetime += deltaTime;
    
    if (lifetime >= maxLifetime) {
        isActive = false;
        isThrown = false;
        return;
    }
    
    if (isThrown) {
        throwTimer += deltaTime;
        
        // 1. 水平线性插值
        float t = throwTimer / totalThrowTime;
        if (t >= 1.0f) {
            t = 1.0f;
            isThrown = false;
            position = targetPosition;
            currentZ = 0.0f;
        } else {
            position.x = startPosition.x + (targetPosition.x - startPosition.x) * t;
            position.y = startPosition.y + (targetPosition.y - startPosition.y) * t;
            
            // 2. 垂直抛物线计算 (模拟 Z 轴)
            // h = 4 * maxH * t * (1 - t)
            float maxHeight = 50.0f;
            currentZ = 4.0f * maxHeight * t * (1.0f - t);
        }
    }
}

void Catnip::checkBoundaries(int mapWidth, int mapHeight) {
    if (!isThrown || !isActive) return;

    // 检查是否到达地面或超出边界
    if (position.y >= mapHeight - 40 || position.x < 0 || position.x > mapWidth) {
        // 落地，停止飞行，激活效果
        isThrown = false;
        velocity = {0, 0};
        if (position.y > mapHeight - 40) position.y = mapHeight - 40;
    }
}

void Catnip::draw() {
    if (!isActive) return;
    
    const float p = 3.0f; // 像素大小
    
    if (isThrown) {
        // 绘制阴影
        DrawCircle(static_cast<int>(position.x), static_cast<int>(position.y), 4, Fade(BLACK, 0.3f));
        
        // 绘制飞行中的猫薄荷 (偏移 Z 轴)
        float drawY = position.y - currentZ;
        
        // 简单的像素叶子形状
        DrawRectangleRec({ position.x - p, drawY - p, 2*p, 2*p }, LIME);
        DrawRectangleRec({ position.x, drawY - 2*p, p, p }, GREEN);
        DrawRectangleRec({ position.x - 2*p, drawY, p, p }, GREEN);
    } else {
        // 落地后的效果
        // 1. 范围波纹
        float pulse = sinf(lifetime * 4.0f) * 5.0f;
        DrawCircleLines(static_cast<int>(position.x), static_cast<int>(position.y), 
                        effectRadius + pulse, Fade(LIME, 0.4f));
        DrawCircle(static_cast<int>(position.x), static_cast<int>(position.y), 
                   effectRadius, Fade(GREEN, 0.1f));
        
        // 2. 落地后的像素叶子
        DrawRectangleRec({ position.x - p, position.y - p, 2*p, 2*p }, GREEN);
        DrawRectangleRec({ position.x + p, position.y, p, p }, DARKGREEN);
        
        // 3. 产生一些微小的向上漂浮的“香气”像素
        for(int i=0; i<3; i++) {
            float particleOff = sinf(lifetime * 2.0f + i) * 10.0f;
            float particleY = position.y - 10.0f - (fmodf(lifetime * 20.0f + i*10, 30.0f));
            DrawRectangleRec({ position.x + particleOff, particleY, 2, 2 }, Fade(LIME, 0.6f));
        }
    }
}

bool Catnip::isInEffectRange(Vector2 catPosition) {
    if (!isActive || isThrown) return false; // 只有落地后才有效
    
    float dx = catPosition.x - position.x;
    float dy = catPosition.y - position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance <= effectRadius;
}

void Catnip::reset() {
    position = {0, 0};
    velocity = {0, 0};
    isThrown = false;
    isActive = false;
    lifetime = 0.0f;
}
