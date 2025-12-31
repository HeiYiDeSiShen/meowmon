#ifndef UIHELPER_HPP
#define UIHELPER_HPP

#include <raylib.h>
#include <string>

class UIHelper {
public:
    // 绘制居中文本
    static void DrawTextCentered(const std::string& text, float y, int fontSize, Color color);
    
    // 绘制按钮
    static bool DrawButton(const std::string& text, Rectangle bounds, Color bgColor, Color textColor);
    
    // 绘制带emoji的文本（避免中文乱码）
    static void DrawTextWithEmoji(const std::string& text, int x, int y, int fontSize, Color color);
    
    // 获取文本宽度
    static int GetTextWidth(const std::string& text, int fontSize);
    
private:
    // 替换中文字符为英文或符号
    static std::string ReplaceChinese(const std::string& text);
};

#endif // UIHELPER_HPP