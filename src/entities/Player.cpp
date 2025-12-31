#include "Player.hpp"
#include "core/ResourceManager.hpp"
#include "Catnip.hpp"
#include <cmath>
#include <iostream>

Player::Player(const std::string& name, Vector2 position)
    : name(name), position(position), velocity({0, 0}), speed(200.0f),
      isMoving(false), currentFrame(0), frameTime(0.0f), animationSpeed(0.1f),
      width(32.0f), height(32.0f), texturePath("assets/sprites/player.png"),
      catnipCooldownTimer(0.0f), catnipCooldownDuration(2.0f), capturedCount(0) {
    
    std::cout << "Player构造函数开始: texturePath=" << texturePath << std::endl;
    
    // 直接创建默认纹理，不依赖ResourceManager
    std::cout << "创建默认玩家方块纹理..." << std::endl;
    Image image = GenImageColor(32, 32, ORANGE);
    sprite = LoadTextureFromImage(image);
    UnloadImage(image);
    texturePath = "default";  // 标记为默认纹理
    
    // 确保设置正确的尺寸
    width = 32.0f;
    height = 32.0f;
    
    std::cout << "默认纹理创建完成: sprite.id=" << sprite.id 
              << " 尺寸=" << width << "x" << height << std::endl;
    
    std::cout << "Player构造函数结束: width=" << width << " height=" << height << std::endl;
    
    // 初始化猫薄荷
    catnip = Catnip();
}

void Player::update(float deltaTime) {
    // 更新位置
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    
    // 更新朝向
    if (velocity.x > 0) facingRight = true;
    else if (velocity.x < 0) facingRight = false;
    
    // 更新动画计时器
    breathTimer += deltaTime;
    if (isMoving) {
        walkTimer += deltaTime * (speed / 20.0f);
    } else {
        walkTimer = 0.0f;
    }
    
    // 更新猫薄荷
    catnip.update(deltaTime);
    
    // 更新冷却时间
    if (catnipCooldownTimer > 0.0f) {
        catnipCooldownTimer -= deltaTime;
        if (catnipCooldownTimer < 0.0f) {
            catnipCooldownTimer = 0.0f;
        }
    }
}

void Player::draw() {
    // --- 核心比例调整 (参考星露谷物语：头大身小，2:3:2 比例) ---
    const float p = 3.0f; // 基础像素大小
    
    // 颜色定义
    Color skinColor = { 255, 200, 150, 255 };
    Color hairColor = { 100, 60, 30, 255 };
    Color shirtColor = { 100, 150, 255, 255 };
    Color pantsColor = { 50, 60, 100, 255 };
    Color shoeColor = { 40, 40, 40, 255 };
    Color shadowColor = { 0, 0, 0, 60 };

    Vector2 center = { position.x + width/2, position.y + height/2 };
    float dir = facingRight ? 1.0f : -1.0f;
    
    // 呼吸与行走动画
    float breath = sinf(GetTime() * 2.0f) * 0.5f;
    float walk = isMoving ? sinf(GetTime() * 12.0f) : 0;

    // 0. 椭圆阴影
    DrawEllipse((int)center.x, (int)(position.y + height - 2), 10, 4, shadowColor);

    // 1. 腿部与脚 (1x2 像素)
    float legY = position.y + height - 6;
    float lOff = walk * 3.0f;
    // 左脚
    DrawRectangleRec({ center.x - 2.0f*p, legY + lOff, p, 4 }, pantsColor);
    DrawRectangleRec({ center.x - 2.0f*p + (dir < 0 ? -1 : 0), legY + lOff + 3, p + 1, 2 }, shoeColor);
    // 右脚
    DrawRectangleRec({ center.x + 1.0f*p, legY - lOff, p, 4 }, pantsColor);
    DrawRectangleRec({ center.x + 1.0f*p + (dir > 0 ? 1 : 0), legY - lOff + 3, p + 1, 2 }, shoeColor);

    // 2. 身体 (更宽厚，带有小手臂)
    float bodyY = legY - 10 + breath;
    DrawRectangleRec({ center.x - 2.5f*p, bodyY, 5*p, 10 }, shirtColor);
    // 手臂
    float armY = bodyY + 2;
    DrawRectangleRec({ center.x + 2.5f*p*dir, armY - lOff*0.5f, p, 6 }, shirtColor);
    DrawRectangleRec({ center.x + 2.5f*p*dir, armY + 5 - lOff*0.5f, p, 2 }, skinColor); // 手

    // 3. 头部 (3x3 像素大头风格)
    float headY = bodyY - 12;
    // 头发
    DrawRectangleRounded({ center.x - 3.5f*p, headY - 2, 7*p, 10 }, 0.4f, 6, hairColor);
    // 脸
    DrawRectangleRec({ center.x - 3.0f*p, headY + 2, 6*p, 8 }, skinColor);
    // 眼睛 (1x1 像素，深色)
    float eyeX = center.x + 1.5f*p*dir;
    DrawRectangleRec({ eyeX, headY + 5, 2, 2 }, { 40, 40, 60, 255 });

    // 4. UI
    DrawText(name.c_str(), (int)(center.x - MeasureText(name.c_str(), 10)/2), (int)(position.y - 15), 10, Fade(BLACK, 0.8f));
    
    catnip.draw();
}

void Player::handleInput() {
    // 重置速度
    velocity = {0, 0};
    isMoving = false;
    
    // 处理WASD输入
    if (IsKeyDown(KEY_W)) {
        velocity.y = -speed;
        isMoving = true;
    }
    if (IsKeyDown(KEY_S)) {
        velocity.y = speed;
        isMoving = true;
    }
    if (IsKeyDown(KEY_A)) {
        velocity.x = -speed;
        isMoving = true;
    }
    if (IsKeyDown(KEY_D)) {
        velocity.x = speed;
        isMoving = true;
    }
    
    // 归一化对角线移动速度
    if (velocity.x != 0 && velocity.y != 0) {
        float length = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        velocity.x = (velocity.x / length) * speed;
        velocity.y = (velocity.y / length) * speed;
    }
    
    // 抛出猫薄荷（空格键或鼠标左键）- 需要冷却
    if ((IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && catnipCooldownTimer <= 0.0f) {
        throwCatnip();
        catnipCooldownTimer = catnipCooldownDuration;  // 重置冷却
    }
}

void Player::checkBoundaries(int mapWidth, int mapHeight) {
    // 边界检测，防止玩家走出地图
    if (position.x < 0) {
        position.x = 0;
    }
    if (position.y < 0) {
        position.y = 0;
    }
    if (position.x + width > mapWidth) {
        position.x = mapWidth - width;
    }
    if (position.y + height > mapHeight) {
        position.y = mapHeight - height;
    }
}

void Player::setPosition(Vector2 position) {
    this->position = position;
}

Vector2 Player::getPosition() const {
    return position;
}

Rectangle Player::getRect() const {
    return {position.x, position.y, width, height};
}

void Player::setSpeed(float speed) {
    this->speed = speed;
}

float Player::getSpeed() const {
    return speed;
}

void Player::setName(const std::string& name) {
    this->name = name;
}

std::string Player::getName() const {
    return name;
}

// 纹理管理方法实现
void Player::setTexturePath(const std::string& path) {
    texturePath = path;
}

void Player::reloadTexture() {
    if (texturePath == "default") {
        // 如果是默认纹理，重新创建白色方块
        Image image = GenImageColor(32, 32, WHITE);
        sprite = LoadTextureFromImage(image);
        UnloadImage(image);
        width = 32.0f;
        height = 32.0f;
    } else {
        // 尝试加载指定路径的纹理
        try {
            sprite = ResourceManager::getInstance().loadTexture(texturePath);
            width = static_cast<float>(sprite.width / 4);
            height = static_cast<float>(sprite.height);
        } catch (const std::exception& e) {
            std::cerr << "无法加载纹理: " << texturePath << " 错误: " << e.what() << std::endl;
            // 加载失败时保持当前纹理
        }
    }
}

std::string Player::getTexturePath() const {
    return texturePath;
}

void Player::throwCatnip() {
    // 获取鼠标位置
    Vector2 mousePosition = GetMousePosition();
    
    // 抛出猫薄荷，朝向鼠标位置
    catnip.throwCatnip(position, mousePosition, 1.0f);
    std::cout << "玩家抛出猫薄荷，目标: (" << mousePosition.x << "," << mousePosition.y << ")" << std::endl;
}

float Player::getCatnipCooldown() const {
    return catnipCooldownTimer;
}

int Player::getCapturedCount() const {
    return capturedCount;
}

void Player::incrementCapturedCount() {
    capturedCount++;
}

bool Player::hasCatnip() const {
    return catnip.getIsActive();
}

Vector2 Player::getCatnipPosition() const {
    return catnip.getPosition();
}

bool Player::isCatnipActive() const {
    return catnip.getIsActive();
}
