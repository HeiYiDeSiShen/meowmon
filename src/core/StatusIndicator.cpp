#include "StatusIndicator.hpp"
#include <cmath>
#include <algorithm>

StatusIndicator::StatusIndicator() 
    : animationTimer(0.0f), pulseTimer(0.0f) {
    // 初始化默认图标配置
    fleeingConfig = {"⚡", YELLOW, 1.2f, 2.0f};
    catnippedConfig = {"🌿", GREEN, 1.5f, 1.5f};
    caughtConfig = {"✨", GOLD, 1.3f, 1.8f};
}

void StatusIndicator::update(float deltaTime) {
    updateAnimations(deltaTime);
    updatePulseEffect(deltaTime);
    
    // 更新所有活跃图标
    for (auto& icon : activeIcons) {
        applyPulseEffect(*icon);
        applyBounceEffect(*icon);
        applyFadeEffect(*icon, deltaTime);
    }
    
    // 移除透明度为0的图标
    activeIcons.erase(
        std::remove_if(activeIcons.begin(), activeIcons.end(),
            [](const std::unique_ptr<StatusIcon>& icon) {
                return icon->alpha <= 0.0f;
            }),
        activeIcons.end()
    );
}

void StatusIndicator::draw() {
    // 这个方法通常不会直接调用，而是通过drawAt()在特定位置绘制
}

void StatusIndicator::drawAt(float x, float y, CatStatus status) {
    if (activeIcons.empty()) return;
    
    // 在指定位置绘制所有活跃图标
    for (const auto& icon : activeIcons) {
        drawIcon(*icon, x, y);
        y += 25; // 垂直堆叠图标
    }
}

void StatusIndicator::showStatus(float x, float y, CatStatus status) {
    // 清除之前的图标
    activeIcons.clear();
    
    // 创建新的状态图标
    StatusIcon* newIcon = createIcon(status);
    if (newIcon) {
        activeIcons.push_back(std::unique_ptr<StatusIcon>(newIcon));
        animationTimer = 0.0f;
        pulseTimer = 0.0f;
    }
}

void StatusIndicator::hideStatus() {
    // 开始淡出动画
    for (auto& icon : activeIcons) {
        icon->alpha = std::max(0.0f, icon->alpha - 0.1f);
    }
}

void StatusIndicator::setFleeingIcon(const std::string& text, Color color) {
    fleeingConfig.text = text;
    fleeingConfig.color = color;
}

void StatusIndicator::setCatnippedIcon(const std::string& text, Color color) {
    catnippedConfig.text = text;
    catnippedConfig.color = color;
}

void StatusIndicator::setCaughtIcon(const std::string& text, Color color) {
    caughtConfig.text = text;
    caughtConfig.color = color;
}

void StatusIndicator::resetAnimation() {
    animationTimer = 0.0f;
    pulseTimer = 0.0f;
    for (auto& icon : activeIcons) {
        icon->scale = 1.0f;
        icon->rotation = 0.0f;
        icon->alpha = 1.0f;
    }
}

void StatusIndicator::updateAnimations(float deltaTime) {
    animationTimer += deltaTime;
    
    // 更新图标动画
    for (auto& icon : activeIcons) {
        // 缩放动画
        float baseScale = 1.0f;
        float pulseScale = 0.1f * std::sin(animationTimer * 4.0f);
        icon->scale = baseScale + pulseScale;
        
        // 旋转动画
        icon->rotation += deltaTime * 30.0f; // 每秒30度
    }
}

void StatusIndicator::updatePulseEffect(float deltaTime) {
    pulseTimer += deltaTime;
}

StatusIcon* StatusIndicator::createIcon(CatStatus status) {
    Color color = getStatusColor(status);
    StatusIcon* icon = new StatusIcon(status, color);
    
    switch (status) {
        case CatStatus::FLEEING: icon->scale = fleeingConfig.baseScale; break;
        case CatStatus::CATNIPPED: icon->scale = catnippedConfig.baseScale; break;
        case CatStatus::CAUGHT: icon->scale = caughtConfig.baseScale; break;
        default: break;
    }
    
    return icon;
}

void StatusIndicator::drawIcon(const StatusIcon& icon, float x, float y) {
    const float s = 3.5f * icon.scale; // 统一使用 3.5f 灵魂像素网格
    Color c = icon.color;
    c.a = (unsigned char)(icon.alpha * 255);
    const float over = 0.5f; // 过度渲染，消除间隙
    
    // 灵魂像素风格状态图标
    switch (icon.type) {
        case CatStatus::FLEEING: { // ⚡ 闪电 (更动感)
            float off = sinf((float)GetTime() * 15.0f) * 2.0f; // 快速抖动
            DrawRectangleRec({x + off, y, s+over, s+over}, c);
            DrawRectangleRec({x + s + off, y, s+over, s+over}, c);
            DrawRectangleRec({x + off, y + s, s+over, s+over}, c);
            DrawRectangleRec({x - s + off, y + s, s+over, s+over}, c);
            DrawRectangleRec({x + off, y + 2*s, s+over, s+over}, c);
            DrawRectangleRec({x - s + off, y + 2*s, s+over, s+over}, c);
            break;
        }
        case CatStatus::CATNIPPED: { // ❤️ 爱心 (更柔和)
            float wave = sinf((float)GetTime() * 3.0f) * 0.1f;
            float ws = s * (1.0f + wave);
            DrawRectangleRec({x-ws, y-ws, ws+over, ws+over}, ColorAlphaBlend(c, PINK, WHITE));
            DrawRectangleRec({x+ws, y-ws, ws+over, ws+over}, ColorAlphaBlend(c, PINK, WHITE));
            DrawRectangleRec({x-2*ws, y, 5*ws+over, ws+over}, ColorAlphaBlend(c, PINK, WHITE));
            DrawRectangleRec({x-ws, y+ws, 3*ws+over, ws+over}, ColorAlphaBlend(c, PINK, WHITE));
            DrawRectangleRec({x, y+2*ws, ws+over, ws+over}, ColorAlphaBlend(c, PINK, WHITE));
            break;
        }
        case CatStatus::CAUGHT: { // ✨ 星星 (更闪耀)
            float rot = (float)GetTime() * 5.0f;
            float s1 = s * (1.0f + sinf(rot) * 0.2f);
            float s2 = s * (1.0f + cosf(rot) * 0.2f);
            DrawRectangleRec({x, y - s1, s+over, s1*2+over}, c); // 纵向
            DrawRectangleRec({x - s2, y, s2*2+over, s+over}, c); // 横向
            // 核心高亮
            DrawRectangleRec({x, y, s+over, s+over}, WHITE);
            break;
        }
        default: break;
    }
}

Color StatusIndicator::getStatusColor(CatStatus status) const {
    switch (status) {
        case CatStatus::FLEEING: return YELLOW;
        case CatStatus::CATNIPPED: return GREEN;
        case CatStatus::CAUGHT: return GOLD;
        default: return WHITE;
    }
}

std::string StatusIndicator::getStatusText(CatStatus status) const {
    switch (status) {
        case CatStatus::FLEEING: return "⚡";
        case CatStatus::CATNIPPED: return "🌿";
        case CatStatus::CAUGHT: return "✨";
        default: return "?";
    }
}

void StatusIndicator::applyPulseEffect(StatusIcon& icon) {
    float pulseIntensity = 0.2f * std::sin(pulseTimer * 3.0f);
    icon.scale = 1.0f + pulseIntensity;
}

void StatusIndicator::applyBounceEffect(StatusIcon& icon) {
    float bounceIntensity = 2.0f * std::sin(animationTimer * 5.0f);
    icon.rotation = bounceIntensity;
}

void StatusIndicator::applyFadeEffect(StatusIcon& icon, float deltaTime) {
    // 如果图标存在时间超过2秒，开始淡出
    if (animationTimer > 2.0f) {
        icon.alpha = std::max(0.0f, icon.alpha - deltaTime * 0.5f);
    }
}