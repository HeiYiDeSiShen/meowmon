#ifndef SETTINGS_MENU_HPP
#define SETTINGS_MENU_HPP

#include <raylib.h>
#include <string>
#include <vector>
#include <functional>

class SettingsMenu {
public:
    struct Setting {
        std::string name;
        std::string description;
        std::function<void()> onClick;
        bool isToggle;
        bool* toggleValue;
        float* sliderValue;
        int* intValue; // 新增 int 指针支持
        float minValue;
        float maxValue;
        
        Setting(const std::string& name, const std::string& desc, std::function<void()> onClick = nullptr)
            : name(name), description(desc), onClick(onClick), isToggle(false), toggleValue(nullptr),
              sliderValue(nullptr), intValue(nullptr), minValue(0.0f), maxValue(1.0f) {}
        
        Setting(const std::string& name, const std::string& desc, bool* toggleValue)
            : name(name), description(desc), onClick(nullptr), isToggle(true), toggleValue(toggleValue),
              sliderValue(nullptr), intValue(nullptr), minValue(0.0f), maxValue(1.0f) {}
        
        Setting(const std::string& name, const std::string& desc, float* sliderValue, float min, float max)
            : name(name), description(desc), onClick(nullptr), isToggle(false), toggleValue(nullptr),
              sliderValue(sliderValue), intValue(nullptr), minValue(min), maxValue(max) {}
        
        Setting(const std::string& name, const std::string& desc, int* intValue, int min, int max)
            : name(name), description(desc), onClick(nullptr), isToggle(false), toggleValue(nullptr),
              sliderValue(nullptr), intValue(intValue), minValue((float)min), maxValue((float)max) {}
    };

private:
    bool isVisible;
    int selectedIndex;
    float menuAlpha;
    float scrollOffset;
    
    // 设置项
    std::vector<Setting> settings;
    
    // 实际设置值
    bool soundEnabled;
    bool musicEnabled;
    bool fullscreenEnabled;
    bool vsyncEnabled;
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    int targetFPS;
    
    // 菜单动画
    float animationTimer;
    bool isAnimatingIn;
    
    // 颜色主题
    Color backgroundColor;
    Color panelColor;
    Color textColor;
    Color highlightColor;
    Color disabledColor;

public:
    SettingsMenu();
    ~SettingsMenu() = default;
    
    // 更新和绘制
    void update(float deltaTime);
    void draw();
    
    // 菜单控制
    void show();
    void hide();
    bool isMenuVisible() const { return isVisible; }
    
    // 设置管理
    void initializeSettings();
    void applySettings();
    void resetToDefaults();
    
    // 输入处理
    void handleInput();
    
    // 获取设置值
    bool isSoundEnabled() const { return soundEnabled; }
    bool isMusicEnabled() const { return musicEnabled; }
    bool isFullscreenEnabled() const { return fullscreenEnabled; }
    bool isVsyncEnabled() const { return vsyncEnabled; }
    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSfxVolume() const { return sfxVolume; }
    int getTargetFPS() const { return targetFPS; }

private:
    // 绘制方法
    void drawBackground();
    void drawMenuPanel();
    void drawSettingsList();
    void drawSettingItem(const Setting& setting, int index, int yPos);
    void drawSlider(const Setting& setting, int x, int y, int width);
    void drawToggle(const Setting& setting, int x, int y);
    void drawButtons();
    
    // 辅助方法
    void updateAnimation(float deltaTime);
    void updateSelection();
    void handleClick(int mouseX, int mouseY);
    Rectangle getSettingBounds(int index) const;
    bool isMouseOverSetting(int index) const;
    
    // 设置回调函数
    void toggleSound();
    void toggleMusic();
    void toggleFullscreen();
    void toggleVsync();
    void changeTargetFPS(int fps);
    
    // 工具方法
    std::string boolToString(bool value) const;
    std::string floatToString(float value, int precision = 1) const;
};

#endif // SETTINGS_MENU_HPP