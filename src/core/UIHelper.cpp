#include "UIHelper.hpp"
#include <algorithm>

void UIHelper::DrawTextCentered(const std::string& text, int y, int fontSize, Color color) {
    int textWidth = GetTextWidth(text, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    DrawText(text.c_str(), x, y, fontSize, color);
}

bool UIHelper::DrawButton(const std::string& text, Rectangle bounds, Color bgColor, Color textColor) {
    // 绘制按钮背景
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);
    
    // 绘制按钮文本
    int textWidth = GetTextWidth(text, 20);
    int textX = (int)bounds.x + ((int)bounds.width - textWidth) / 2;
    int textY = (int)bounds.y + ((int)bounds.height - 20) / 2;
    DrawText(text.c_str(), textX, textY, 20, textColor);
    
    // 检查鼠标悬停和点击
    Vector2 mousePoint = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePoint, bounds);
    
    if (hovered) {
        DrawRectangleRec(bounds, Fade(WHITE, 0.2f));
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            return true;
        }
    }
    
    return false;
}

void UIHelper::DrawTextWithEmoji(const std::string& text, int x, int y, int fontSize, Color color) {
    // 简单的emoji支持，直接绘制原始文本
    DrawText(text.c_str(), x, y, fontSize, color);
}

int UIHelper::GetTextWidth(const std::string& text, int fontSize) {
    return MeasureText(text.c_str(), fontSize);
}

std::string UIHelper::ReplaceChinese(const std::string& text) {
    // 简单的中文字符替换，避免乱码
    std::string result = text;
    
    // 替换常见中文
    result = std::string(result).replace(result.find("小花"), 4, "XiaoHua");
    result = std::string(result).replace(result.find("橘子"), 4, "Orange");
    result = std::string(result).replace(result.find("黑猫"), 4, "BlackCat");
    result = std::string(result).replace(result.find("白猫"), 4, "WhiteCat");
    
    return result;
}