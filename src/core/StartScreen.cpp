#include "StartScreen.hpp"
#include <iostream>

StartScreen::StartScreen() 
    : titleY(-50), subtitleY(GetScreenHeight() + 50), buttonY(GetScreenHeight() + 100), timer(0.0f),
      entranceAnim(0.0f), hoverProgress(0.0f), arrowOffset(0.0f), mouseGlowPos({0, 0}) {
    // 初始化背景粒子
    for (int i = 0; i < 50; i++) {
        particles.push_back({
            {(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)},
            {(float)GetRandomValue(-20, 20) / 10.0f, (float)GetRandomValue(10, 50) / 10.0f},
            (float)GetRandomValue(2, 5),
            (float)GetRandomValue(5, 10) / 10.0f,
            GetRandomValue(0, 1) == 0 ? SKYBLUE : WHITE
        });
    }
}

void StartScreen::update(float deltaTime) {
    timer += deltaTime;
    
    // 入场动画平滑过渡 (0 -> 1)
    if (entranceAnim < 1.0f) {
        entranceAnim += deltaTime * 1.5f;
        if (entranceAnim > 1.0f) entranceAnim = 1.0f;
    }
    
    // 鼠标光晕跟随逻辑（带惯性）
    Vector2 targetMouse = GetMousePosition();
    mouseGlowPos.x += (targetMouse.x - mouseGlowPos.x) * 10.0f * deltaTime;
    mouseGlowPos.y += (targetMouse.y - mouseGlowPos.y) * 10.0f * deltaTime;
    
    // 更新粒子（保留一些环境氛围）
    for (auto& p : particles) {
        p.position.x += p.velocity.x;
        p.position.y += p.velocity.y;
        
        if (p.position.y > 600) {
            p.position.y = -10;
            p.position.x = GetRandomValue(0, 800);
        }
    }
}

void StartScreen::draw() {
    // 1. Tailwind 风格深色背景
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), 
                          (Color){15, 23, 42, 255}, (Color){30, 41, 59, 255}); // Slate-900 to Slate-800
    
    // 2. 随鼠标移动的动态光晕 (Glow Effect)
    DrawCircleGradient(mouseGlowPos.x, mouseGlowPos.y, 300, Fade(SKYBLUE, 0.15f), BLANK);
    
    // 3. 绘制环境粒子
    for (const auto& p : particles) {
        DrawCircleV(p.position, p.radius, Fade(p.color, p.alpha * 0.3f));
    }

    // 计算入场动画偏移 (Slide + Blur simulation)
    float slideY = (1.0f - entranceAnim) * 50.0f;
    float alpha = entranceAnim;

    // 4. 绘制标题 (Tailwind Typo: Tracking-tight, font-bold)
    const char* title = "MEOWMON";
    float titleYPos = 180 + slideY;
    
    // 在标题两旁增加装饰小猫 (展现新画风)
    float decorY = titleYPos - 20 + sinf(timer * 2.0f) * 10.0f;
    DrawCircleV({GetScreenWidth()/2.0f - 240, decorY}, 30, Fade(SKYBLUE, 0.2f * alpha)); // 光晕
    DrawCircleV({GetScreenWidth()/2.0f + 240, decorY}, 30, Fade(PINK, 0.2f * alpha));
    
    // 5. 绘制标题文本
    UIHelper::DrawTextCentered(title, titleYPos, 80, Fade(WHITE, alpha));

    // 6. 绘制副标题 (Tailwind: text-slate-400)
    UIHelper::DrawTextCentered("Catch 'em all with soul and pixel", 
                              titleYPos + 90, 22, Fade((Color){148, 163, 184, 255}, alpha));

    // 6. Call-to-action 按钮
    drawStartButton();

    // 7. 底部提示 (Tailwind: text-slate-500)
    UIHelper::DrawTextCentered("Press [SPACE] or Click to begin journey", 
                              GetScreenHeight() - 60, 16, Fade((Color){100, 116, 139, 255}, alpha));
}

void StartScreen::drawStartButton() {
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = 400 + (1.0f - entranceAnim) * 30.0f;
    
    const char* btnText = "Start Hunting";
    int fontSize = 28;
    int textWidth = MeasureText(btnText, fontSize);
    
    Rectangle hitBox = { centerX - textWidth/2.0f - 20, centerY - 10, (float)textWidth + 80, 50 };
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), hitBox);
    
    // 更新悬停动画进度
    float dt = GetFrameTime();
    if (isHovered) {
        hoverProgress += dt * 8.0f;
        arrowOffset += dt * 15.0f;
    } else {
        hoverProgress -= dt * 6.0f;
        arrowOffset -= dt * 10.0f;
    }
    hoverProgress = fmaxf(0.0f, fminf(1.0f, hoverProgress));
    arrowOffset = fmaxf(0.0f, fminf(10.0f, arrowOffset));

    // 绘制按钮文字
    Color textColor = isHovered ? WHITE : (Color){226, 232, 240, 255}; // Slate-200 to White
    UIHelper::DrawTextCentered(btnText, centerY, fontSize, Fade(textColor, entranceAnim));
    
    // 箭头图标位移效果 (->)
    float arrowX = centerX + textWidth/2.0f + 15 + arrowOffset;
    DrawText("->", arrowX, centerY + 2, fontSize, Fade(textColor, entranceAnim));

    // 悬停时的下划线延展 (Tailwind: border-b-2)
    float lineY = centerY + fontSize + 5;
    float lineWidth = (textWidth + 40) * hoverProgress;
    DrawRectangle(centerX - lineWidth/2.0f, lineY, lineWidth, 2, SKYBLUE);
}

void StartScreen::drawBackground() {
    // 逻辑已整合到 draw() 中，此处留空或移除
}

void StartScreen::drawTitle() {
    // 逻辑已整合到 draw() 中，此处留空或移除
}

bool StartScreen::shouldStartGame() {
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = 400;
    
    const char* btnText = "Start Hunting";
    int fontSize = 28;
    int textWidth = MeasureText(btnText, fontSize);
    
    Rectangle hitBox = { centerX - textWidth/2.0f - 20, centerY - 10, (float)textWidth + 80, 50 };
    
    bool clicked = CheckCollisionPointRec(GetMousePosition(), hitBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    return clicked || IsKeyPressed(KEY_SPACE);
}
