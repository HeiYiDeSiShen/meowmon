#include "Cat.hpp"
#include "core/ResourceManager.hpp"
#include <cmath>
#include <iostream>
#include <memory>
#include <random>

Cat::Cat(const std::string& name, Vector2 position, CatType type)
    : name(name), position(position), velocity({0, 0}), speed(50.0f),
      isMoving(false), facingRight(true), breathTimer(0.0f), walkTimer(0.0f),
      isCaught(false), currentFrame(0), frameTime(0.0f), 
      animationSpeed(8.0f), width(40.0f), height(30.0f), 
      legLength(10.0f), bodyFatness(1.0f), isFluffy(false),
      sprite({0}), texturePath(""), rd(nullptr), gen(nullptr),
      aiChangeDirectionTimer(0.0f), aiChangeDirectionInterval(2.0f + (rand() % 3)),
      type(type), smartMovePattern(0), state(CatState::NORMAL),
      catnipTimer(0), stateTimer(0), color(WHITE),
      catnipEffectTimer(0.0f), baseEffectTime(10.0f), catnipPosition({0, 0}),
      earAngle(0.0f), eyeSize(1.0f), whiskerLength(1.0f), tailStyle(0.0f),
      isShiny(false), personality(CatPersonality::NORMAL), personalityTimer(0.0f) {
    
    // 初始化随机数生成器
    rd = std::make_unique<std::random_device>();
    gen = std::make_unique<std::mt19937>((*rd)());
    
    // 初始化动画计时器
    breathTimer = static_cast<float>(gen->operator()() % 100) / 10.0f;
    walkTimer = 0.0f;
    blinkTimer = 2.0f + static_cast<float>(gen->operator()() % 30) / 10.0f;
    blinkDuration = 0.15f;
    isBlinking = false;
    tailWagTimer = static_cast<float>(gen->operator()() % 100) / 10.0f;
    
    // 随机印象属性
    earAngle = 0.0f;
    eyeSize = 0.9f + static_cast<float>(gen->operator()() % 3) / 10.0f;
    whiskerLength = 0.8f + static_cast<float>(gen->operator()() % 5) / 10.0f;
    tailStyle = 0.8f + static_cast<float>(gen->operator()() % 5) / 10.0f;
    
    // 随机稀有度 (5% 概率为闪光猫)
    isShiny = (gen->operator()() % 100) < 5;
    if (isShiny) {
        // 闪光猫特效颜色
        int effectType = gen->operator()() % 3;
        if (effectType == 0) shinyEffectColor = GOLD;
        else if (effectType == 1) shinyEffectColor = SKYBLUE;
        else shinyEffectColor = PINK;
    }

    // 随机性格
    int pType = gen->operator()() % 4;
    if (pType == 1) personality = CatPersonality::COWARD;
    else if (pType == 2) personality = CatPersonality::GREEDY;
    else if (pType == 3) personality = CatPersonality::CURIOUS;
    else personality = CatPersonality::NORMAL;

    // 根据品种设置基础属性
    if (type == CatType::PERSIAN) {
        speed = 220.0f;       // 跑得比人快一点点
        baseEffectTime = 15.0f;
        color = ORANGE;
    } else if (type == CatType::SIAMESE) {
        speed = 280.0f;      // 矫健的品种
        baseEffectTime = 8.0f;
        color = BROWN;
    } else if (type == CatType::MAINE_COON) {
        speed = 240.0f;      // 体型大，速度中等偏上
        baseEffectTime = 12.0f;
        color = DARKGRAY;
    } else if (type == CatType::RAGDOLL) {
        speed = 210.0f;       // 温顺，速度略高于人
        baseEffectTime = 18.0f;
        color = LIGHTGRAY;
    } else if (type == CatType::BENGAL) {
        speed = 350.0f;      // 猎豹般的爆发力
        baseEffectTime = 6.0f;
        color = YELLOW;
    } else {
        speed = 250.0f;
        baseEffectTime = 10.0f;
        color = WHITE;
    }
    
    // 初始化状态
    state = CatState::NORMAL;
    catnipEffectTimer = 0.0f;
    catnipPosition = {0, 0};
    
    // 设置初始随机方向
    changeRandomDirection();
    
    std::cout << "猫咪创建: " << name << " 位置(" << position.x << "," << position.y << ")" << std::endl;
    
    // 初始化状态指示器
    statusIndicator = std::make_unique<StatusIndicator>();
    
    // 加载纹理
    std::string spritePath = "";
    if (type == CatType::PERSIAN) spritePath = "assets/sprites/cat_persian.png";
    else if (type == CatType::SIAMESE) spritePath = "assets/sprites/cat_siamese.png";
    else if (type == CatType::MAINE_COON) spritePath = "assets/sprites/cat_maine_coon.png";
    else if (type == CatType::RAGDOLL) spritePath = "assets/sprites/cat_ragdoll.png";
    else if (type == CatType::BENGAL) spritePath = "assets/sprites/cat_bengal.png";

    if (!spritePath.empty()) {
        sprite = ResourceManager::getInstance().loadTexture(spritePath);
        if (sprite.id > 0) {
            std::cout << "猫咪纹理加载成功: " << spritePath << std::endl;
        } else {
            // 尝试备用路径
            spritePath = "../" + spritePath;
            sprite = ResourceManager::getInstance().loadTexture(spritePath);
            if (sprite.id > 0) {
                std::cout << "猫咪纹理加载成功 (备用路径): " << spritePath << std::endl;
            }
        }
    }

    if (sprite.id == 0) {
        std::cout << "猫咪创建: " << name << " (使用程序化绘制)" << std::endl;
    } else {
        std::cout << "猫咪创建: " << name << " (使用纹理: " << spritePath << ")" << std::endl;
    }
}

Cat::Cat(const std::string& name, float x, float y, CatType type)
    : Cat(name, {x, y}, type) {}

Cat::~Cat() {
    // 资源由 unique_ptr 自动管理
}

// 移动构造函数
Cat::Cat(Cat&& other) noexcept
    : name(std::move(other.name)), position(other.position), velocity(other.velocity),
      speed(other.speed), isMoving(other.isMoving), facingRight(other.facingRight), 
      isCaught(other.isCaught), currentFrame(other.currentFrame), frameTime(other.frameTime), 
      animationSpeed(other.animationSpeed), width(other.width), height(other.height),
      legLength(other.legLength), bodyFatness(other.bodyFatness), isFluffy(other.isFluffy),
      sprite(other.sprite), texturePath(std::move(other.texturePath)),
      rd(std::move(other.rd)), gen(std::move(other.gen)), type(other.type), state(other.state),
      catnipEffectTimer(other.catnipEffectTimer), baseEffectTime(other.baseEffectTime),
      catnipPosition(other.catnipPosition), catnipTimer(other.catnipTimer),
      stateTimer(other.stateTimer), smartMovePattern(other.smartMovePattern),
      aiChangeDirectionTimer(other.aiChangeDirectionTimer), 
      aiChangeDirectionInterval(other.aiChangeDirectionInterval),
      earAngle(other.earAngle), eyeSize(other.eyeSize), 
      whiskerLength(other.whiskerLength), tailStyle(other.tailStyle),
      statusIndicator(std::move(other.statusIndicator)), color(other.color) {
    
    // 将源对象的资源置空
    other.sprite.id = 0;
}

// 移动赋值运算符
Cat& Cat::operator=(Cat&& other) noexcept {
    if (this != &other) {
        // 移动数据
        name = std::move(other.name);
        position = other.position;
        velocity = other.velocity;
        speed = other.speed;
        isMoving = other.isMoving;
        facingRight = other.facingRight;
        isCaught = other.isCaught;
        currentFrame = other.currentFrame;
        frameTime = other.frameTime;
        animationSpeed = other.animationSpeed;
        width = other.width;
        height = other.height;
        legLength = other.legLength;
        bodyFatness = other.bodyFatness;
        isFluffy = other.isFluffy;
        sprite = other.sprite;
        texturePath = std::move(other.texturePath);
        rd = std::move(other.rd);
        gen = std::move(other.gen);
        type = other.type;
        state = other.state;
        catnipEffectTimer = other.catnipEffectTimer;
        baseEffectTime = other.baseEffectTime;
        catnipPosition = other.catnipPosition;
        catnipTimer = other.catnipTimer;
        stateTimer = other.stateTimer;
        smartMovePattern = other.smartMovePattern;
        aiChangeDirectionTimer = other.aiChangeDirectionTimer;
        aiChangeDirectionInterval = other.aiChangeDirectionInterval;
        earAngle = other.earAngle;
        eyeSize = other.eyeSize;
        whiskerLength = other.whiskerLength;
        tailStyle = other.tailStyle;
        statusIndicator = std::move(other.statusIndicator);
        color = other.color;
        
        // 将源对象的资源置空
        other.sprite.id = 0;
    }
    return *this;
}

void Cat::update(float deltaTime, Vector2 playerPosition, Vector2 catnipPosition, bool hasCatnip, int capturedCount) {
    if (isCaught) return;
    
    updateTimers(deltaTime);
    updateState(playerPosition, catnipPosition, hasCatnip, capturedCount, deltaTime);
    updateMovement(deltaTime);
    updateAI(deltaTime, playerPosition);
    updateStatusIndicator(deltaTime);
    
    // 更新位置
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
    
    // 更新转向
    if (std::abs(velocity.x) > 0.1f) {
        facingRight = (velocity.x > 0);
    }
    
    // 更新动画计时器
    breathTimer += deltaTime;
    if (isMoving) {
        walkTimer += deltaTime * (speed / 20.0f);
    } else {
        walkTimer = 0;
    }
    
    // 检查边界
    checkBoundaries(800, 600); // 假设屏幕大小为800x600
}

void Cat::draw() {
    if (isCaught) return;

    // --- 核心比例调整 (参考星露谷物语：短小精悍，可爱的侧身/正脸混合) ---
    const float p = 3.0f;
    Color shadowColor = { 0, 0, 0, 50 };
    
    // 颜色配置 (更符合星露谷的柔和调色盘)
    Color bodyColor = color;
    Color eyeColor = (state == CatState::CATNIPPED) ? PINK : Color{ 40, 40, 40, 255 };
    
    switch (type) {
        case CatType::PERSIAN: bodyColor = { 245, 245, 240, 255 }; break;
        case CatType::SIAMESE: bodyColor = { 230, 200, 180, 255 }; break;
        case CatType::MAINE_COON: bodyColor = { 100, 80, 60, 255 }; break;
        case CatType::RAGDOLL: bodyColor = { 240, 240, 250, 255 }; break;
        default: break;
    }

    Vector2 center = { position.x + width/2, position.y + height/2 };
    float dir = facingRight ? 1.0f : -1.0f;
    float walk = isMoving ? sinf((float)GetTime() * 10.0f) : 0;
    float breath = sinf((float)GetTime() * 3.0f) * 0.4f;

    // 0. 阴影
    DrawEllipse((int)center.x, (int)(position.y + height - 2), 12, 4, shadowColor);

    // 1. 身体 (横向的长方形，带圆角)
    float bodyW = 12 * p;
    float bodyH = 8 * p + breath;
    float bodyY = position.y + height - bodyH - 6;
    DrawRectangleRounded({ center.x - (bodyW/2), bodyY, bodyW, bodyH }, 0.5f, 6, bodyColor);

    // 2. 头部 (紧贴身体前方)
    float headSize = 7 * p;
    float headX = center.x + (bodyW/2 - 2*p) * dir;
    float headY = bodyY - 2*p;
    DrawRectangleRounded({ headX - headSize/2, headY, headSize, headSize }, 0.3f, 4, bodyColor);

    // 3. 耳朵 (尖尖的小三角)
    DrawTriangle({ headX - 3*p, headY + 2 }, { headX - 4*p, headY - 3*p }, { headX - p, headY + 2 }, bodyColor);
    DrawTriangle({ headX + p, headY + 2 }, { headX + 4*p, headY - 3*p }, { headX + 3*p, headY + 2 }, bodyColor);

    // 4. 眼睛 (星露谷风格：简单的黑点)
    DrawCircle((int)(headX - 1.5f*p), (int)(headY + 3*p), (state == CatState::CATNIPPED ? 2.5f : 1.5f), eyeColor);
    DrawCircle((int)(headX + 1.5f*p), (int)(headY + 3*p), (state == CatState::CATNIPPED ? 2.5f : 1.5f), eyeColor);

    // 5. 尾巴 (星露谷风格：Q弹的曲线)
    float tailX = center.x - (bodyW/2) * dir;
    float tailY = bodyY + bodyH/2;
    float tailWag = sinf((float)GetTime() * 8.0f) * 5.0f;
    for(int i=0; i<4; i++) {
        DrawCircle((int)(tailX - i*2*p*dir), (int)(tailY - i*p + tailWag*(i/4.0f)), 3.5f - i*0.5f, bodyColor);
    }

    // 6. 短腿 (1x1 像素)
    float legY = position.y + height - 4;
    DrawRectangleRec({ center.x - 4*p, legY + walk*2, p, p }, bodyColor);
    DrawRectangleRec({ center.x + 2*p, legY - walk*2, p, p }, bodyColor);

    // 7. UI
    DrawText(name.c_str(), (int)(center.x - MeasureText(name.c_str(), 10)/2), (int)(position.y - 15), 10, Fade(BLACK, 0.7f));
    
    drawStatusIndicator();
}

void Cat::updateTimers(float deltaTime) {
    // 更新各种计时器
    if (catnipTimer > 0) catnipTimer -= deltaTime;
    if (stateTimer > 0) stateTimer -= deltaTime;
    if (aiChangeDirectionTimer > 0) aiChangeDirectionTimer -= deltaTime;
    if (catnipEffectTimer > 0) catnipEffectTimer -= deltaTime;
    
    // 更新动画计时器
    breathTimer += deltaTime;
    walkTimer += deltaTime;
    tailWagTimer += deltaTime;
    
    // 眨眼逻辑
    blinkTimer -= deltaTime;
    if (blinkTimer <= 0) {
        if (!isBlinking) {
            isBlinking = true;
            blinkTimer = blinkDuration;
        } else {
            isBlinking = false;
            blinkTimer = 2.0f + static_cast<float>(gen->operator()() % 40) / 10.0f; // 随机下次眨眼时间
        }
    }
}

void Cat::updateAI(float deltaTime, Vector2 playerPos) {
    if (isCaught) {
        velocity = {0, 0};
        isMoving = false;
        return;
    }

    // 根据不同性格调整 AI 行为
    float fleeMultiplier = 1.0f;
    float catnipMultiplier = 1.0f;

    // 性格影响基础参数
    if (personality == CatPersonality::COWARD) {
        fleeMultiplier *= 1.5f; // 胆小鬼跑得更快
    }
    if (personality == CatPersonality::GREEDY) {
        catnipMultiplier *= 1.2f; // 贪吃的猫跑向猫薄荷更快
    }

    switch (state) {
        case CatState::NORMAL:
            normalAI(deltaTime, playerPos);
            break;
        case CatState::FLEEING:
            fleeAI(deltaTime * fleeMultiplier);
            break;
        case CatState::CATNIPPED:
            catnipAI(deltaTime * catnipMultiplier);
            break;
        case CatState::CAUGHT:
            velocity = {0, 0};
            isMoving = false;
            break;
        default:
            break;
    }
}

void Cat::normalAI(float deltaTime, Vector2 playerPos) {
    // 随机移动逻辑
    stateTimer -= deltaTime;
    if (stateTimer <= 0) {
        if (gen->operator()() % 100 < 30) {
            isMoving = true;
            changeRandomDirection();
        } else {
            isMoving = false;
            velocity = {0, 0};
        }
        stateTimer = (float)(gen->operator()() % 3 + 1); // 1-3秒切换一次状态
    }

    // 更新AI方向改变计时器
    aiChangeDirectionTimer += deltaTime;
    
    // 性格逻辑：好奇的猫会尝试靠近静止的玩家
    if (personality == CatPersonality::CURIOUS) {
        personalityTimer += deltaTime;
        if (personalityTimer > 3.0f) {
            // 检查玩家是否静止 (这里简化为距离判断)
            float dx = position.x - playerPos.x;
            float dy = position.y - playerPos.y;
            float distToPlayer = std::sqrt(dx * dx + dy * dy);
            
            if (distToPlayer < 200.0f && distToPlayer > 50.0f) {
                // 向玩家移动
                Vector2 dir = {playerPos.x - position.x, playerPos.y - position.y};
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 0) {
                    velocity.x = (dir.x / len) * speed * 0.5f;
                    velocity.y = (dir.y / len) * speed * 0.5f;
                    isMoving = true;
                }
            } else if (gen->operator()() % 100 < 20) {
                isMoving = true;
                changeRandomDirection();
            }
            personalityTimer = 0.0f;
        }
    }

    // 随机改变方向
    if (aiChangeDirectionTimer >= aiChangeDirectionInterval) {
        changeRandomDirection();
        aiChangeDirectionTimer = 0.0f;
        aiChangeDirectionInterval = 1.0f + static_cast<float>(gen->operator()() % 30) / 10.0f;
    }
    
    // 随机停止/移动
    if (gen->operator()() % 100 < 5) {
        isMoving = !isMoving;
        if (!isMoving) {
            velocity = {0, 0};
        } else {
            changeRandomDirection();
        }
    }
    
    // 保持移动
    if (isMoving && (velocity.x == 0 && velocity.y == 0)) {
        changeRandomDirection();
    }
}

void Cat::fleeAI(float deltaTime) {
    // 计算远离玩家的方向
    Vector2 fleeDirection = {position.x - catnipPosition.x, position.y - catnipPosition.y};
    
    // 归一化
    float length = std::sqrt(fleeDirection.x * fleeDirection.x + fleeDirection.y * fleeDirection.y);
    if (length > 0) {
        fleeDirection.x /= length;
        fleeDirection.y /= length;
    }
    
    // 根据品种和性格设置逃跑速度
    float fleeMultiplier = 1.2f; // 基础逃跑速度倍率
    if (type == CatType::BENGAL) fleeMultiplier = 1.6f;
    else if (type == CatType::SIAMESE) fleeMultiplier = 1.4f;
    else if (type == CatType::RAGDOLL) fleeMultiplier = 1.1f;

    if (personality == CatPersonality::COWARD) {
        fleeMultiplier *= 1.4f; // 胆小鬼逃跑动力更强
    }
    
    // 设置高速逃跑
    velocity.x = fleeDirection.x * speed * fleeMultiplier;
    velocity.y = fleeDirection.y * speed * fleeMultiplier;
    isMoving = true;
    
    // 聪明猫咪会尝试绕圈逃跑
    if (type == CatType::SIAMESE && smartMovePattern % 3 == 0) {
        velocity.x += fleeDirection.y * speed * 0.5f;
        velocity.y -= fleeDirection.x * speed * 0.5f;
    }
    
    smartMovePattern++;
    
    // 逃跑一段时间后恢复正常
    if (catnipEffectTimer <= 0.0f) {
        setState(CatState::NORMAL);
    }
}

void Cat::catnipAI(float deltaTime) {
    // 向猫薄荷位置缓慢移动
    Vector2 toCatnip = {catnipPosition.x - position.x, catnipPosition.y - position.y};
    
    float length = std::sqrt(toCatnip.x * toCatnip.x + toCatnip.y * toCatnip.y);
    if (length > 0) {
        toCatnip.x /= length;
        toCatnip.y /= length;
    }
    
    // 品种和性格影响沉迷移动速度
    float catnipMultiplier = 0.5f;
    if (type == CatType::RAGDOLL) catnipMultiplier = 0.3f;
    else if (type == CatType::BENGAL) catnipMultiplier = 0.7f;

    if (personality == CatPersonality::GREEDY) {
        catnipMultiplier *= 1.2f; // 贪吃的猫跑向猫薄荷更快
    }
    
    // 沉迷状态移动缓慢
    velocity.x = toCatnip.x * speed * catnipMultiplier;
    velocity.y = toCatnip.y * speed * catnipMultiplier;
    isMoving = true;
    
    // 如果靠近猫薄荷，停止移动（沉迷中）
    if (length < 20.0f) {
        velocity = {0, 0};
        isMoving = false;
    }
    
    // 沉迷时间结束，恢复正常
    if (catnipEffectTimer <= 0.0f) {
        setState(CatState::NORMAL);
        std::cout << "猫咪恢复清醒: " << name << std::endl;
    }
}

void Cat::changeRandomDirection() {
    // 随机方向
    int direction = gen->operator()() % 8; // 8个方向
    
    float angle = (direction * 45.0f) * (PI / 180.0f); // 转换为弧度
    
    velocity.x = std::cos(angle) * speed;
    velocity.y = std::sin(angle) * speed;
    
    isMoving = true;
}

void Cat::checkBoundaries(int mapWidth, int mapHeight) {
    // 边界检测，碰到边界反弹
    if (position.x < 0) {
        position.x = 0;
        velocity.x = -velocity.x; // 反弹
    }
    if (position.y < 0) {
        position.y = 0;
        velocity.y = -velocity.y; // 反弹
    }
    if (position.x + width > mapWidth) {
        position.x = mapWidth - width;
        velocity.x = -velocity.x; // 反弹
    }
    if (position.y + height > mapHeight) {
        position.y = mapHeight - height;
        velocity.y = -velocity.y; // 反弹
    }
}

bool Cat::checkCollision(const Rectangle& playerRect) {
    Rectangle catRect = {position.x, position.y, width, height};
    return CheckCollisionRecs(catRect, playerRect);
}

void Cat::setPosition(Vector2 pos) {
    position = pos;
}

void Cat::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

Rectangle Cat::getRect() const {
    return {position.x, position.y, width, height};
}

void Cat::setCaught(bool caught) {
    isCaught = caught;
    if (caught) {
        velocity = {0, 0};
        isMoving = false;
        setState(CatState::CAUGHT);
    }
}

void Cat::setSpeed(float speed) {
    this->speed = speed;
}

void Cat::setName(const std::string& name) {
    this->name = name;
}

void Cat::setTexturePath(const std::string& path) {
    texturePath = path;
}

void Cat::reloadTexture() {
    if (texturePath == "default") {
        Image image = GenImageColor(32, 32, PINK);
        sprite = LoadTextureFromImage(image);
        UnloadImage(image);
        width = 32.0f;
        height = 32.0f;
    } else {
        try {
            // 这里可以加载自定义猫咪纹理
            // sprite = ResourceManager::getInstance().loadTexture(texturePath);
            // width = static_cast<float>(sprite.width);
            // height = static_cast<float>(sprite.height);
        } catch (const std::exception& e) {
            std::cerr << "无法加载猫咪纹理: " << texturePath << " 错误: " << e.what() << std::endl;
        }
    }
}

std::string Cat::getTexturePath() const {
    return texturePath;
}

// 品种相关方法实现
void Cat::setCatType(CatType type) {
    this->type = type;
    
    // 根据新品种更新属性
    if (type == CatType::PERSIAN) {
        speed = 220.0f;
        baseEffectTime = 15.0f;
        color = ORANGE;
    } else if (type == CatType::SIAMESE) {
        speed = 280.0f;
        baseEffectTime = 8.0f;
        color = BROWN;
    } else if (type == CatType::MAINE_COON) {
        speed = 240.0f;
        baseEffectTime = 12.0f;
        color = DARKGRAY;
    } else if (type == CatType::RAGDOLL) {
        speed = 210.0f;
        baseEffectTime = 18.0f;
        color = LIGHTGRAY;
    } else if (type == CatType::BENGAL) {
        speed = 350.0f;
        baseEffectTime = 6.0f;
        color = YELLOW;
    }
}

CatType Cat::getCatType() const {
    return type;
}

std::string Cat::getCatTypeName() const {
    if (type == CatType::PERSIAN) return "波斯猫";
    if (type == CatType::SIAMESE) return "暹罗猫";
    if (type == CatType::MAINE_COON) return "缅因猫";
    if (type == CatType::RAGDOLL) return "布偶猫";
    if (type == CatType::BENGAL) return "孟加拉猫";
    return "未知";
}

// 获取猫咪基础速度
float Cat::getBaseSpeed() const {
    return speed;
}

// GIF功能已完全移除

void Cat::updateState(Vector2 playerPosition, Vector2 catnipPosition, bool hasCatnip, int capturedCount, float deltaTime) {
    // 如果被抓到，不更新状态
    if (isCaught) return;
    
    // 计算与玩家的距离
    float dx = position.x - playerPosition.x;
    float dy = position.y - playerPosition.y;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);
    
    // 计算与猫薄荷的距离
    float cdx = position.x - catnipPosition.x;
    float cdy = position.y - catnipPosition.y;
    float distanceToCatnip = std::sqrt(cdx * cdx + cdy * cdy);
    
    // 根据品种调整吸引范围
    float attractRange = 100.0f;
    if (type == CatType::BENGAL) attractRange = 120.0f;  // 孟加拉猫范围更大
    else if (type == CatType::RAGDOLL) attractRange = 90.0f;  // 布偶猫范围更小
    
    // 根据性格调整吸引范围
    if (personality == CatPersonality::GREEDY) attractRange *= 1.5f;
    else if (personality == CatPersonality::COWARD) attractRange *= 0.8f;
    
    // 逃跑触发范围
    float alertRange = 120.0f;
    if (personality == CatPersonality::COWARD) alertRange = 200.0f; // 胆小猫逃得更早
    
    // 沉迷时间加成
    float bonusTime = capturedCount * 0.5f;
    float totalEffectTime = baseEffectTime + bonusTime;
    if (personality == CatPersonality::GREEDY) totalEffectTime *= 1.3f; // 贪吃猫沉迷更久

    // 状态切换逻辑
    if (state == CatState::NORMAL) {
        // 检查猫薄荷
        if (hasCatnip && distanceToCatnip < attractRange) {
            setState(CatState::CATNIPPED);
            catnipEffectTimer = totalEffectTime;
            this->catnipPosition = catnipPosition;
            std::cout << "猫咪被猫薄荷吸引: " << getCatTypeName() << " " << name << " 沉迷时间: " << totalEffectTime << "秒" << std::endl;
        }
        // 检查玩家 (逃跑)
        else if (distanceToPlayer < alertRange) {
            setState(CatState::FLEEING);
            catnipEffectTimer = 2.0f; // 逃跑 2 秒
            this->catnipPosition = playerPosition; // 远离玩家
            std::cout << "猫咪逃跑: " << getCatTypeName() << " " << name << std::endl;
        }
        // 好奇性格：如果玩家没动，慢慢靠近
        else if (personality == CatPersonality::CURIOUS && distanceToPlayer < 200.0f) {
            personalityTimer += deltaTime;
            if (personalityTimer > 2.0f) { // 盯着看2秒后靠近
                Vector2 toPlayer = {playerPosition.x - position.x, playerPosition.y - position.y};
                float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
                if (len > 50.0f) {
                    velocity.x = (toPlayer.x / len) * speed * 0.4f;
                    velocity.y = (toPlayer.y / len) * speed * 0.4f;
                    isMoving = true;
                }
            }
        } else {
            personalityTimer = 0.0f;
        }
    } else if (state == CatState::FLEEING) {
        // 逃跑中如果遇到猫薄荷，胆小猫可能不会理会，贪吃猫可能会切换状态
        if (hasCatnip && distanceToCatnip < attractRange * 0.5f && personality == CatPersonality::GREEDY) {
            setState(CatState::CATNIPPED);
            catnipEffectTimer = totalEffectTime * 0.7f; // 逃跑中切换，效果减弱
            this->catnipPosition = catnipPosition;
        }
    }
}

void Cat::setState(CatState newState) {
    if (state != newState) {
        state = newState;
        stateTimer = 0.0f;
        
        // 更新状态指示器
        if (statusIndicator) {
            Vector2 indicatorPos = {position.x + width/2, position.y - 20};
            if (state == CatState::FLEEING) {
                statusIndicator->showStatus(indicatorPos.x, indicatorPos.y, CatStatus::FLEEING);
            } else if (state == CatState::CATNIPPED) {
                statusIndicator->showStatus(indicatorPos.x, indicatorPos.y, CatStatus::CATNIPPED);
            } else if (state == CatState::CAUGHT) {
                statusIndicator->showStatus(indicatorPos.x, indicatorPos.y, CatStatus::CAUGHT);
            } else {
                statusIndicator->hideStatus(); // NORMAL 状态隐藏指示器
            }
        }
    }
}

void Cat::updateMovement(float deltaTime) {
    // 基础移动逻辑，根据当前状态调整移动
    if (isCaught) return;
    
    // 注意：位置更新现在统一在 update() 中进行，这里只负责逻辑处理
}

void Cat::updateStatusIndicator(float deltaTime) {
    if (statusIndicator) {
        statusIndicator->update(deltaTime);
    }
}

void Cat::drawStatusIndicator() {
    if (statusIndicator && statusIndicator->isActive()) {
        // 在猫咪上方绘制状态指示器
        Vector2 indicatorPos = {position.x + width/2, position.y - 15};
        
        CatStatus currentStatus = CatStatus::NORMAL;
        if (state == CatState::FLEEING) currentStatus = CatStatus::FLEEING;
        else if (state == CatState::CATNIPPED) currentStatus = CatStatus::CATNIPPED;
        else if (state == CatState::CAUGHT) currentStatus = CatStatus::CAUGHT;
        
        statusIndicator->drawAt(indicatorPos.x, indicatorPos.y, currentStatus);
    }
}

float Cat::getCatnipTimeRemaining() const {
    if (state == CatState::CATNIPPED) {
        return catnipEffectTimer;
    }
    return 0.0f;
}
