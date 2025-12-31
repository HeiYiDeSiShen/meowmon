#include "SettingsMenu.hpp"
#include <raylib.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

SettingsMenu::SettingsMenu() 
    : isVisible(false), selectedIndex(0), menuAlpha(0.0f), scrollOffset(0.0f),
      animationTimer(0.0f), isAnimatingIn(false),
      soundEnabled(true), musicEnabled(true), fullscreenEnabled(false), vsyncEnabled(true),
      masterVolume(1.0f), musicVolume(0.8f), sfxVolume(0.8f), targetFPS(60),
      backgroundColor({0, 0, 0, 200}), panelColor({40, 40, 40, 240}),
      textColor(WHITE), highlightColor(SKYBLUE), disabledColor(GRAY) {
    
    initializeSettings();
}

void SettingsMenu::initializeSettings() {
    settings.clear();
    
    // 音频设置
    settings.push_back(Setting("Sound Effects", "Enable/disable sound effects", &soundEnabled));
    settings.push_back(Setting("Music", "Enable/disable background music", &musicEnabled));
    settings.push_back(Setting("Master Volume", "Overall volume level", &masterVolume, 0.0f, 1.0f));
    settings.push_back(Setting("Music Volume", "Background music volume", &musicVolume, 0.0f, 1.0f));
    settings.push_back(Setting("SFX Volume", "Sound effects volume", &sfxVolume, 0.0f, 1.0f));
    
    // 显示设置
    settings.push_back(Setting("Fullscreen", "Toggle fullscreen mode", &fullscreenEnabled));
    settings.push_back(Setting("VSync", "Enable/disable vertical sync", &vsyncEnabled));
    settings.push_back(Setting("Target FPS", "Target frames per second", &targetFPS, 30, 144));
    
    // 更多测试选项以验证滚动
    settings.push_back(Setting("Difficulty", "Game difficulty level", nullptr));
    settings.push_back(Setting("Language", "Switch between CN/EN", nullptr));
    settings.push_back(Setting("Auto Save", "Enable periodic saving", &vsyncEnabled)); // 复用变量测试
    
    // 游戏设置
    settings.push_back(Setting("Reset to Defaults", "Reset all settings to default values", [this]() {
        resetToDefaults();
    }));
    
    settings.push_back(Setting("Back to Game", "Close settings and return to game", [this]() {
        hide();
    }));
}

void SettingsMenu::update(float deltaTime) {
    if (!isVisible && menuAlpha <= 0.0f) return;
    
    updateAnimation(deltaTime);
    
    if (isVisible) {
        handleInput();
        updateSelection();
    }
}

void SettingsMenu::draw() {
    if (menuAlpha <= 0.0f) return;
    
    drawBackground();
    drawMenuPanel();
    drawSettingsList();
    drawButtons();
}

void SettingsMenu::show() {
    isVisible = true;
    isAnimatingIn = true;
    animationTimer = 0.0f;
    selectedIndex = 0;
    scrollOffset = 0.0f;
}

void SettingsMenu::hide() {
    isVisible = false;
    isAnimatingIn = false;
    animationTimer = 0.0f;
}

void SettingsMenu::handleInput() {
    // 鼠标滚轮滚动
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        scrollOffset = std::max(0.0f, std::min((float)settings.size() - 8, scrollOffset - mouseWheel));
    }

    if (IsKeyPressed(KEY_UP)) {
        selectedIndex = std::max(0, selectedIndex - 1);
        // 确保选中项在可见区域内
        if (selectedIndex < (int)scrollOffset) {
            scrollOffset = (float)selectedIndex;
        }
    }
    if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex = std::min((int)settings.size() - 1, selectedIndex + 1);
        // 确保选中项在可见区域内
        int visibleItems = 8;
        if (selectedIndex >= (int)scrollOffset + visibleItems) {
            scrollOffset = (float)(selectedIndex - visibleItems + 1);
        }
    }
    
    if (IsKeyPressed(KEY_LEFT)) {
        Setting& setting = settings[selectedIndex];
        if (setting.sliderValue) {
            *setting.sliderValue = std::max(setting.minValue, *setting.sliderValue - 0.1f);
        } else if (setting.intValue) {
            *setting.intValue = std::max((int)setting.minValue, *setting.intValue - 10);
        }
    }
    
    if (IsKeyPressed(KEY_RIGHT)) {
        Setting& setting = settings[selectedIndex];
        if (setting.sliderValue) {
            *setting.sliderValue = std::min(setting.maxValue, *setting.sliderValue + 0.1f);
        } else if (setting.intValue) {
            *setting.intValue = std::min((int)setting.maxValue, *setting.intValue + 10);
        }
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < (int)settings.size(); i++) {
                if (isMouseOverSetting(i)) {
                    selectedIndex = i;
                    break;
                }
            }
        }
        
        Setting& setting = settings[selectedIndex];
        if (setting.isToggle && setting.toggleValue) {
            *setting.toggleValue = !*setting.toggleValue;
            if (setting.name == "Sound Effects") toggleSound();
            if (setting.name == "Music") toggleMusic();
            if (setting.name == "Fullscreen") toggleFullscreen();
            if (setting.name == "VSync") toggleVsync();
        } else if (setting.onClick) {
            setting.onClick();
        }
    }
}

void SettingsMenu::updateAnimation(float deltaTime) {
    const float animationSpeed = 5.0f;
    
    if (isAnimatingIn) {
        animationTimer += deltaTime * animationSpeed;
        menuAlpha = std::min(1.0f, animationTimer);
        if (animationTimer >= 1.0f) {
            isAnimatingIn = false;
        }
    } else if (!isVisible) {
        animationTimer -= deltaTime * animationSpeed;
        menuAlpha = std::max(0.0f, animationTimer);
    }
}

void SettingsMenu::updateSelection() {
    // 逻辑已移动到 handleInput 中处理
}

void SettingsMenu::drawBackground() {
    Color bgColor = backgroundColor;
    bgColor.a = (unsigned char)(bgColor.a * menuAlpha);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bgColor);
}

void SettingsMenu::drawMenuPanel() {
    int panelWidth = 600;
    int panelHeight = 450;
    int panelX = (GetScreenWidth() - panelWidth) / 2;
    int panelY = (GetScreenHeight() - panelHeight) / 2;
    
    // 阴影
    DrawRectangle(panelX + 8, panelY + 8, panelWidth, panelHeight, Fade(BLACK, 0.4f * menuAlpha));
    
    // 主面板 (带渐变和圆角)
    DrawRectangleRounded({ (float)panelX, (float)panelY, (float)panelWidth, (float)panelHeight }, 0.08f, 12, Fade(panelColor, menuAlpha));
    DrawRectangleRoundedLines({ (float)panelX, (float)panelY, (float)panelWidth, (float)panelHeight }, 0.08f, 12, Fade(highlightColor, menuAlpha));
    
    // 标题区域
    DrawRectangleRounded({ (float)panelX + 20, (float)panelY + 15, (float)panelWidth - 40, 45 }, 0.2f, 10, Fade(BLACK, 0.3f * menuAlpha));
    const char* title = "SYSTEM SETTINGS / 系统设置";
    int titleWidth = MeasureText(title, 22);
    DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 28, 22, Fade(WHITE, menuAlpha));
}

void SettingsMenu::drawSettingsList() {
    int panelWidth = 600;
    int panelHeight = 450;
    int panelX = (GetScreenWidth() - panelWidth) / 2;
    int panelY = (GetScreenHeight() - panelHeight) / 2;
    
    int contentX = panelX + 30;
    int contentY = panelY + 80;
    int contentWidth = panelWidth - 60;
    int visibleCount = 6;
    int itemHeight = 55;
    
    for (int i = 0; i < visibleCount; i++) {
        int idx = (int)scrollOffset + i;
        if (idx >= (int)settings.size()) break;
        
        int yPos = contentY + i * itemHeight;
        drawSettingItem(settings[idx], idx, yPos);
    }
}

void SettingsMenu::drawSettingItem(const Setting& setting, int index, int yPos) {
    int panelWidth = 600;
    int panelX = (GetScreenWidth() - panelWidth) / 2;
    int contentX = panelX + 40;
    int itemWidth = panelWidth - 80;
    bool isSelected = (index == selectedIndex);
    
    // 选中背景
    if (isSelected) {
        DrawRectangleRounded({ (float)contentX - 10, (float)yPos - 5, (float)itemWidth + 20, 50 }, 0.2f, 8, Fade(highlightColor, 0.15f * menuAlpha));
        DrawRectangleRec({ (float)contentX - 10, (float)yPos - 5, 4, 50 }, highlightColor);
    }
    
    // 标题与描述
    DrawText(setting.name.c_str(), contentX, yPos + 2, 18, isSelected ? highlightColor : textColor);
    DrawText(setting.description.c_str(), contentX, yPos + 24, 13, Fade(textColor, 0.6f * menuAlpha));
    
    // 控制组件
    int controlX = panelX + 360;
    if (setting.isToggle && setting.toggleValue) {
        bool val = *setting.toggleValue;
        DrawRectangleRounded({ (float)controlX, (float)yPos + 10, 50, 24 }, 0.5f, 10, val ? GREEN : DARKGRAY);
        DrawCircle(controlX + (val ? 38 : 12), yPos + 22, 9, WHITE);
    } else if (setting.sliderValue) {
        float val = *setting.sliderValue;
        float percent = (val - setting.minValue) / (setting.maxValue - setting.minValue);
        DrawRectangle(controlX, yPos + 21, 150, 4, DARKGRAY);
        DrawRectangle(controlX, yPos + 21, 150 * percent, 4, highlightColor);
        DrawCircle(controlX + 150 * percent, yPos + 23, 7, WHITE);
    } else if (setting.intValue) {
        DrawText(TextFormat("< %d >", *setting.intValue), controlX + 40, yPos + 12, 18, highlightColor);
    } else if (setting.onClick) {
        DrawRectangleRounded({ (float)controlX, (float)yPos + 8, 120, 30 }, 0.2f, 10, isSelected ? highlightColor : Fade(GRAY, 0.3f));
        DrawText("EXECUTE", controlX + 30, yPos + 16, 14, isSelected ? BLACK : WHITE);
    }
}

void SettingsMenu::drawSlider(const Setting& setting, int x, int y, int width) {
    float value = *setting.sliderValue;
    float normalizedValue = (value - setting.minValue) / (setting.maxValue - setting.minValue);
    
    // 绘制滑块轨道
    DrawRectangle(x, y + 8, width, 4, disabledColor);
    DrawRectangle(x, y + 8, (int)(width * normalizedValue), 4, highlightColor);
    
    // 绘制滑块手柄
    int handleX = x + (int)(width * normalizedValue) - 4;
    DrawRectangle(handleX, y + 4, 8, 12, highlightColor);
    
    // 绘制数值
    std::string valueStr = setting.name == "Target FPS" ? 
        std::to_string((int)value) : floatToString(value, 1);
    DrawText(valueStr.c_str(), x + width + 10, y, 14, textColor);
}

void SettingsMenu::drawToggle(const Setting& setting, int x, int y) {
    bool value = *setting.toggleValue;
    Color toggleColor = value ? GREEN : RED;
    
    DrawRectangle(x, y, 40, 20, toggleColor);
    DrawRectangleLines(x, y, 40, 20, WHITE);
    
    // 绘制开关状态
    int circleX = value ? x + 25 : x + 5;
    DrawCircle(circleX, y + 10, 8, WHITE);
    
    // 绘制状态文字
    const char* status = value ? "ON" : "OFF";
    DrawText(status, x + 50, y, 14, textColor);
}

void SettingsMenu::drawButtons() {
    int panelWidth = 600;
    int panelHeight = 500;
    int panelX = (GetScreenWidth() - panelWidth) / 2;
    int panelY = (GetScreenHeight() - panelHeight) / 2;
    
    int buttonY = panelY + panelHeight - 50;
    int buttonWidth = 100;
    int buttonHeight = 30;
    int spacing = 20;
    
    int totalWidth = 2 * buttonWidth + spacing;
    int startX = panelX + (panelWidth - totalWidth) / 2;
    
    // 应用按钮
    Color applyColor = GREEN;
    applyColor.a = (unsigned char)(applyColor.a * menuAlpha);
    DrawRectangle(startX, buttonY, buttonWidth, buttonHeight, applyColor);
    DrawText("Apply", startX + 25, buttonY + 8, 16, WHITE);
    
    // 取消按钮
    Color cancelColor = RED;
    cancelColor.a = (unsigned char)(cancelColor.a * menuAlpha);
    DrawRectangle(startX + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight, cancelColor);
    DrawText("Cancel", startX + buttonWidth + spacing + 20, buttonY + 8, 16, WHITE);
    
    // 检查按钮点击
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, { (float)startX, (float)buttonY, (float)buttonWidth, (float)buttonHeight })) {
            applySettings();
            hide();
        } else if (CheckCollisionPointRec(mousePos, { (float)(startX + buttonWidth + spacing), (float)buttonY, (float)buttonWidth, (float)buttonHeight })) {
            hide();
        }
    }
}

void SettingsMenu::applySettings() {
    // 应用音频设置
    if (soundEnabled) {
        SetMasterVolume(masterVolume);
    } else {
        SetMasterVolume(0.0f);
    }
    
    // 应用显示设置
    if (vsyncEnabled) {
        SetWindowState(FLAG_VSYNC_HINT);
    } else {
        ClearWindowState(FLAG_VSYNC_HINT);
    }
    
    SetTargetFPS(targetFPS);
    
    // 应用全屏设置
    if (fullscreenEnabled != IsWindowFullscreen()) {
        ToggleFullscreen();
    }
}

void SettingsMenu::resetToDefaults() {
    soundEnabled = true;
    musicEnabled = true;
    fullscreenEnabled = false;
    vsyncEnabled = true;
    masterVolume = 1.0f;
    musicVolume = 0.8f;
    sfxVolume = 0.8f;
    targetFPS = 60;
    
    initializeSettings();
}

void SettingsMenu::toggleSound() {
    // 这里可以添加具体的音效开关逻辑
    std::cout << "Sound effects: " << (soundEnabled ? "ON" : "OFF") << std::endl;
}

void SettingsMenu::toggleMusic() {
    // 这里可以添加具体的音乐开关逻辑
    std::cout << "Music: " << (musicEnabled ? "ON" : "OFF") << std::endl;
}

void SettingsMenu::toggleFullscreen() {
    // 全屏切换会在applySettings中处理
    std::cout << "Fullscreen: " << (fullscreenEnabled ? "ON" : "OFF") << std::endl;
}

void SettingsMenu::toggleVsync() {
    // 垂直同步会在applySettings中处理
    std::cout << "VSync: " << (vsyncEnabled ? "ON" : "OFF") << std::endl;
}

void SettingsMenu::changeTargetFPS(int fps) {
    std::cout << "Target FPS: " << fps << std::endl;
}

Rectangle SettingsMenu::getSettingBounds(int index) const {
    int panelWidth = 600;
    int panelHeight = 500;
    int panelX = (GetScreenWidth() - panelWidth) / 2;
    int panelY = (GetScreenHeight() - panelHeight) / 2;
    
    int contentX = panelX + 40;
    int contentY = panelY + 80 + (index - scrollOffset) * 40;
    int contentWidth = panelWidth - 80;
    int itemHeight = 40;
    
    return { (float)contentX - 10, (float)contentY - 5, (float)contentWidth, (float)itemHeight };
}

bool SettingsMenu::isMouseOverSetting(int index) const {
    Vector2 mousePos = GetMousePosition();
    Rectangle bounds = getSettingBounds(index);
    return CheckCollisionPointRec(mousePos, bounds);
}

std::string SettingsMenu::boolToString(bool value) const {
    return value ? "true" : "false";
}

std::string SettingsMenu::floatToString(float value, int precision) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}