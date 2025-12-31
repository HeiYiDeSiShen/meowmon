#ifndef STATUS_INDICATOR_HPP
#define STATUS_INDICATOR_HPP

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>

enum class CatStatus {
    NORMAL,
    FLEEING,
    CATNIPPED,
    CAUGHT
};

struct StatusIcon {
    CatStatus type;
    Color color;
    float scale;
    float rotation;
    float alpha;
    
    StatusIcon(CatStatus type, Color color) 
        : type(type), color(color), scale(1.0f), rotation(0.0f), alpha(1.0f) {}
};

class StatusIndicator {
private:
    std::vector<std::unique_ptr<StatusIcon>> activeIcons;
    float animationTimer;
    float pulseTimer;
    
    // 图标配置
    struct IconConfig {
        std::string text;
        Color color;
        float baseScale;
        float animationSpeed;
    };
    
    IconConfig fleeingConfig;
    IconConfig catnippedConfig;
    IconConfig caughtConfig;
    
public:
    StatusIndicator();
    ~StatusIndicator() = default;
    
    // 更新和绘制
    void update(float deltaTime);
    void draw();
    void drawAt(float x, float y, CatStatus status);
    
    // 状态管理
    void showStatus(float x, float y, CatStatus status);
    void hideStatus();
    bool isActive() const { return !activeIcons.empty(); }
    
    // 配置方法
    void setFleeingIcon(const std::string& text, Color color);
    void setCatnippedIcon(const std::string& text, Color color);
    void setCaughtIcon(const std::string& text, Color color);
    
    // 动画控制
    void setAnimationSpeed(float speed) { animationTimer = speed; }
    void resetAnimation();
    
private:
    // 内部方法
    void updateAnimations(float deltaTime);
    void updatePulseEffect(float deltaTime);
    StatusIcon* createIcon(CatStatus status);
    void drawIcon(const StatusIcon& icon, float x, float y);
    Color getStatusColor(CatStatus status) const;
    std::string getStatusText(CatStatus status) const;
    
    // 特效方法
    void applyPulseEffect(StatusIcon& icon);
    void applyBounceEffect(StatusIcon& icon);
    void applyFadeEffect(StatusIcon& icon, float deltaTime);
};

#endif // STATUS_INDICATOR_HPP