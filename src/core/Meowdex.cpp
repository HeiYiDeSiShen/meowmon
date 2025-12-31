#include "Meowdex.hpp"
#include "ResourceManager.hpp"
#include "UIHelper.hpp"
#include "rlgl.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

Meowdex::Meowdex() : isVisible(false), isDetailMode(false), selectedType(CatType::PERSIAN), detailAnimationTimer(0.0f), is3DMode(true), rotationAngle(0.0f), feedbackTimer(0.0f), feedbackMessage(""), catBounceY(0.0f) {
    // 初始化 3D 相机
    camera.position = { 0.0f, 2.0f, 10.0f }; // 调整相机位置，更适合观察
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 35.0f; // 稍微缩小视野
    camera.projection = CAMERA_PERSPECTIVE;
    
    initEntries();
    loadProgress();
}

void Meowdex::toggleVisibility() {
    isVisible = !isVisible;
    if (!isVisible) isDetailMode = false; // 关闭图鉴时重置详情模式
}

void Meowdex::setVisible(bool visible) {
    isVisible = visible;
    if (!isVisible) isDetailMode = false;
}

void Meowdex::update(float deltaTime) {
    if (!isVisible) return;
    
    // 更新反馈状态
    if (feedbackTimer > 0) feedbackTimer -= deltaTime;
    if (catBounceY > 0) {
        catBounceY -= deltaTime * 10.0f;
        if (catBounceY < 0) catBounceY = 0;
    }

    if (isDetailMode) {
        detailAnimationTimer += deltaTime;
        
        // 3D 旋转逻辑
        if (is3DMode) {
            rotationAngle += deltaTime * 40.0f; // 稍微加快旋转速度
            if (IsKeyDown(KEY_LEFT)) rotationAngle -= deltaTime * 150.0f;
            if (IsKeyDown(KEY_RIGHT)) rotationAngle += deltaTime * 150.0f;
        }

        // 处理鼠标点击交互
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            
            // 调试信息：点击位置
            // std::cout << "Mouse Click: " << mousePos.x << ", " << mousePos.y << std::endl;
            
            // 统一判定区域 (470-730, 100-550)
            if (mousePos.x >= 460 && mousePos.x <= 740) {
                if (mousePos.y >= 340 && mousePos.y <= 390) interact("feed");
                else if (mousePos.y >= 400 && mousePos.y <= 450) interact("play");
                else if (mousePos.y >= 460 && mousePos.y <= 510) isDetailMode = false;
            }
            
            // 3D/2D 切换按钮 (固定在左下方)
            if (CheckCollisionPointRec(mousePos, {40, 530, 200, 50})) {
                is3DMode = !is3DMode;
            }
            
            // 点击左侧大猫区域触发摸摸
            if (mousePos.x >= 50 && mousePos.x <= 450 && mousePos.y >= 100 && mousePos.y <= 500) {
                interact("pet");
            }
        }

        // 快捷键保持
        if (IsKeyPressed(KEY_F)) interact("feed");
        if (IsKeyPressed(KEY_P)) interact("play");
        if (IsKeyPressed(KEY_T)) is3DMode = !is3DMode;
        
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE)) isDetailMode = false;
    } else {
        // 列表模式下的点击检测
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            int index = 0;
            for (auto& pair : entries) {
                auto& entry = pair.second;
                float y = 60 + index * 100;
                // 增大点击判定区域
                if (CheckCollisionPointRec(mousePos, {50, y, 700, 90}) && entry.caughtCount > 0) {
                    selectedType = pair.first;
                    isDetailMode = true;
                    detailAnimationTimer = 0.0f;
                    is3DMode = true; // 默认开启 3D 增加惊喜感
                    break;
                }
                index++;
            }
        }
    }
}

void Meowdex::interact(const std::string& action) {
    auto& entry = entries[selectedType];
    feedbackTimer = 1.0f; // 显示 1 秒反馈
    catBounceY = 1.5f;    // 向上跳一下
    
    if (action == "feed") {
        entry.feedCount++;
        entry.affection = std::min(100.0f, entry.affection + 2.5f);
        feedbackMessage = "好吃！好感度 +2.5";
    } else if (action == "play") {
        entry.playCount++;
        entry.affection = std::min(100.0f, entry.affection + 4.0f);
        feedbackMessage = "开心！好感度 +4.0";
    } else if (action == "pet") {
        entry.affection = std::min(100.0f, entry.affection + 0.5f);
        feedbackMessage = "呼噜噜... 好感度 +0.5";
    }
    saveProgress();
}

void Meowdex::saveProgress() {
    std::ofstream file("meowdex_data.sav");
    if (!file.is_open()) return;

    for (auto const& [type, entry] : entries) {
        file << static_cast<int>(type) << " " 
             << entry.caughtCount << " " 
             << (entry.discoveredShiny ? 1 : 0) << " "
             << entry.affection << " "
             << entry.feedCount << " "
             << entry.playCount << " ";
        
        file << entry.discoveredPersonalities.size() << " ";
        for (auto p : entry.discoveredPersonalities) {
            file << static_cast<int>(p) << " ";
        }
        file << "\n";
    }
    file.close();
}

void Meowdex::loadProgress() {
    std::ifstream file("meowdex_data.sav");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        int typeInt, caughtCount, isShiny;
        float affection;
        int feedCount, playCount;
        
        if (!(ss >> typeInt >> caughtCount >> isShiny >> affection >> feedCount >> playCount)) continue;

        CatType type = (CatType)typeInt;
        if (entries.count(type)) {
            entries[type].caughtCount = caughtCount;
            entries[type].discoveredShiny = (isShiny == 1);
            entries[type].affection = affection;
            entries[type].feedCount = feedCount;
            entries[type].playCount = playCount;

            size_t pSize;
            if (ss >> pSize) {
                for (size_t i = 0; i < pSize; ++i) {
                    int pInt;
                    if (ss >> pInt) {
                        entries[type].discoveredPersonalities.push_back((CatPersonality)pInt);
                    }
                }
            }
        }
    }
    file.close();
}

void Meowdex::draw() {
    if (!isVisible) return;

    Font font = ResourceManager::getInstance().getFont("assets/fonts/chinese_font.ttf", 20);
    bool hasFont = font.texture.id != 0;

    if (isDetailMode) {
        drawDetailView(font, hasFont);
    } else {
        drawListView(font, hasFont);
    }
}

void Meowdex::drawListView(Font font, bool hasFont) {
    // 绘制半透明背景
    DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.85f));
    
    float startX = 50, startY = 60;

    // 标题
    const char* title = "MEOW-DEX (猫咪图鉴)";
    if (hasFont) DrawTextEx(font, title, {startX, 20}, 24, 2, GOLD);
    else DrawText(title, (int)startX, 20, 24, GOLD);

    int index = 0;
    for (auto const& [type, entry] : entries) {
        float y = startY + index * 100;
        
        // 绘制卡片背景
        DrawRectangle(startX, y, 700, 90, Fade(GRAY, 0.2f));
        DrawRectangleLines(startX, y, 700, 90, entry.caughtCount > 0 ? SKYBLUE : DARKGRAY);

        // 品种名称
        std::string nameText = entry.speciesName + (entry.caughtCount > 0 ? "" : " (未发现)");
        if (hasFont) DrawTextEx(font, nameText.c_str(), {startX + 20, y + 15}, 20, 1, entry.caughtCount > 0 ? WHITE : GRAY);
        
        // 抓获数量
        std::string countText = "已抓获: " + std::to_string(entry.caughtCount);
        if (hasFont) DrawTextEx(font, countText.c_str(), {startX + 20, y + 45}, 16, 1, GOLD);

        // 描述 (只有抓过才显示)
        if (entry.caughtCount > 0) {
            if (hasFont) DrawTextEx(font, entry.description.c_str(), {startX + 150, y + 15}, 14, 1, LIGHTGRAY);
            
            // 绘制发现的标识
            float iconX = startX + 150;
            float iconY = y + 55;
            
            if (entry.discoveredShiny) {
                DrawPoly({iconX, iconY}, 5, 8, 0, GOLD);
                if (hasFont) DrawTextEx(font, "闪光", {iconX + 12, iconY - 7}, 12, 1, GOLD);
                iconX += 60;
            }
            
            for (auto p : entry.discoveredPersonalities) {
                Color pColor = SKYBLUE;
                const char* pName = "普通";
                if (p == CatPersonality::COWARD) { pColor = SKYBLUE; pName = "胆小"; }
                else if (p == CatPersonality::GREEDY) { pColor = LIME; pName = "贪吃"; }
                else if (p == CatPersonality::CURIOUS) { pColor = PINK; pName = "好奇"; }
                
                DrawCircleV({iconX, iconY}, 5, pColor);
                if (hasFont) DrawTextEx(font, pName, {iconX + 10, iconY - 7}, 12, 1, pColor);
                iconX += 50;
            }
            
            // 点击提示
            if (hasFont) DrawTextEx(font, "点击查看详情 >", {startX + 580, y + 65}, 14, 1, Fade(WHITE, 0.5f));
        }

        index++;
    }

    // 关闭提示
    const char* closeHint = "按 [M] 关闭图鉴";
    if (hasFont) DrawTextEx(font, closeHint, {330, 560}, 18, 1, WHITE);
}

void Meowdex::drawDetailView(Font font, bool hasFont) {
    const auto& entry = entries[selectedType];
    
    // 背景
    DrawRectangle(0, 0, 800, 600, Fade(DARKGRAY, 0.95f));
    DrawRectangleLinesEx({20, 20, 760, 560}, 2, SKYBLUE);

    // 标题
    std::string title = "猫咪详情: " + entry.speciesName;
    if (hasFont) DrawTextEx(font, title.c_str(), {40, 40}, 30, 2, GOLD);

    // --- 核心展示区：放大版猫咪 ---
    float breath = sinf(detailAnimationTimer * 3.0f) * 0.2f + catBounceY;
    
    if (!is3DMode) {
        // --- 2D 像素风展示 ---
        float centerX = 250, centerY = 300;
        float scale = 6.0f; 
        float pixelBounce = (sinf(detailAnimationTimer * 3.0f) * 5.0f) - (catBounceY * 20.0f);
        
        // 身体
        DrawRectangle(centerX - 40, centerY - 20 + pixelBounce, 80, 60, WHITE);
        // 头部
        DrawRectangle(centerX - 35, centerY - 60 + pixelBounce * 0.5f, 50, 45, WHITE);
        // 耳朵
        DrawTriangle({centerX - 35, centerY - 60 + pixelBounce * 0.5f}, {centerX - 45, centerY - 80 + pixelBounce * 0.5f}, {centerX - 20, centerY - 60 + pixelBounce * 0.5f}, WHITE);
        DrawTriangle({centerX + 15, centerY - 60 + pixelBounce * 0.5f}, {centerX + 25, centerY - 80 + pixelBounce * 0.5f}, {centerX + 5, centerY - 60 + pixelBounce * 0.5f}, WHITE);
        // 眼睛
        if (sinf(detailAnimationTimer * 2.0f) > -0.8f) {
            DrawCircle(centerX - 20, centerY - 40 + pixelBounce * 0.5f, 5, BLACK);
            DrawCircle(centerX + 5, centerY - 40 + pixelBounce * 0.5f, 5, BLACK);
        }
    } else {
        // --- 3D 体素风展示 ---
        BeginMode3D(camera);
            rlPushMatrix();
                rlRotatef(rotationAngle, 0, 1, 0); 
                
                Color catColor = WHITE;
                switch(selectedType) {
                    case CatType::SIAMESE: catColor = { 235, 200, 175, 255 }; break;
                    case CatType::PERSIAN: catColor = WHITE; break;
                    case CatType::MAINE_COON: catColor = { 100, 80, 60, 255 }; break;
                    case CatType::RAGDOLL: catColor = { 245, 245, 250, 255 }; break;
                    case CatType::BENGAL: catColor = { 210, 140, 60, 255 }; break;
                    default: catColor = WHITE; break;
                }
                
                // 绘制猫咪模型 (使用 breath 和 catBounceY)
                DrawCube({ 0, breath, 0 }, 4.0f, 3.0f, 2.5f, catColor);
                DrawCubeWires({ 0, breath, 0 }, 4.0f, 3.0f, 2.5f, Fade(BLACK, 0.3f));
                DrawCube({ 1.5f, 1.5f + breath * 1.2f, 0 }, 2.5f, 2.2f, 2.2f, catColor);
                DrawCubeWires({ 1.5f, 1.5f + breath * 1.2f, 0 }, 2.5f, 2.2f, 2.2f, Fade(BLACK, 0.3f));
                
                // 如果有反馈，显示心形粒子 (简化为红色方块)
                if (feedbackTimer > 0) {
                    DrawCube({ 0, 4.0f + feedbackTimer, 0 }, 0.5f, 0.5f, 0.5f, PINK);
                }
            rlPopMatrix();
        EndMode3D();
    }

    // 反馈消息
    if (feedbackTimer > 0 && hasFont) {
        DrawTextEx(font, feedbackMessage.c_str(), {150, 400}, 20, 1, PINK);
    }

    // --- 互动面板 (简化为按钮) ---
    float panelX = 470;
    DrawRectangle(panelX - 10, 100, 280, 450, Fade(BLACK, 0.4f));
    
    if (hasFont) {
        DrawTextEx(font, "心情指数", {panelX, 120}, 20, 1, SKYBLUE);
        
        // 好感度条
        DrawRectangle(panelX, 155, 260, 20, BLACK);
        DrawRectangle(panelX, 155, 260 * (entry.affection / 100.0f), 20, PINK);
        DrawTextEx(font, TextFormat("%.1f%%", entry.affection), {panelX + 110, 157}, 16, 1, WHITE);
        
        // 按钮
        auto drawBtn = [&](Rectangle rec, const char* text, Color col) {
            bool hover = CheckCollisionPointRec(GetMousePosition(), rec);
            DrawRectangleRec(rec, hover ? ColorBrightness(col, 0.2f) : col);
            DrawRectangleLinesEx(rec, 2, WHITE);
            DrawTextEx(font, text, {rec.x + 85, rec.y + 10}, 18, 1, WHITE);
        };

        drawBtn({panelX, 350, 260, 40}, "喂食", DARKGREEN);
        drawBtn({panelX, 410, 260, 40}, "玩耍", DARKBLUE);
        drawBtn({panelX, 470, 260, 40}, "返回", MAROON);
        
        // 3D/2D 切换按钮
        Rectangle toggleRec = {40, 540, 150, 30};
        bool toggleHover = CheckCollisionPointRec(GetMousePosition(), toggleRec);
        DrawRectangleRec(toggleRec, toggleHover ? GRAY : DARKGRAY);
        DrawTextEx(font, is3DMode ? "切换到 2D" : "切换到 3D", {toggleRec.x + 25, toggleRec.y + 7}, 16, 1, WHITE);
    }

    // 描述文本
    if (hasFont) {
        DrawTextEx(font, entry.description.c_str(), {40, 480}, 18, 1, WHITE);
        DrawTextEx(font, "点击猫咪可以摸摸它哦！", {40, 450}, 16, 1, LIGHTGRAY);
    }
}

void Meowdex::initEntries() {
    entries[CatType::PERSIAN] = {CatType::PERSIAN, "波斯猫", 0, false, {}, "高贵优雅，毛发蓬松。虽然动作缓慢，但对猫薄荷有着惊人的执着。"};
    entries[CatType::SIAMESE] = {CatType::SIAMESE, "暹罗猫", 0, false, {}, "聪明伶俐，好奇心强。它们擅长绕开玩家的捕捉，需要一点耐心。"};
    entries[CatType::MAINE_COON] = {CatType::MAINE_COON, "缅因猫", 0, false, {}, "猫中巨人，性格温顺。虽然体型庞大，但跑起来却意外地轻盈。"};
    entries[CatType::RAGDOLL] = {CatType::RAGDOLL, "布偶猫", 0, false, {}, "像布娃娃一样柔软。它们非常容易被猫薄荷吸引，是最容易捕捉的品种。"};
    entries[CatType::BENGAL] = {CatType::BENGAL, "孟加拉猫", 0, false, {}, "充满野性活力。速度极快，对危险感知敏锐，是捕捉者的终极挑战。"};
}

void Meowdex::recordCapture(const Cat& cat) {
    auto it = entries.find(cat.getCatType());
    if (it != entries.end()) {
        it->second.caughtCount++;
        if (cat.getIsShiny()) it->second.discoveredShiny = true;
        
        // 记录性格
        auto& pList = it->second.discoveredPersonalities;
        if (std::find(pList.begin(), pList.end(), cat.getPersonality()) == pList.end()) {
            pList.push_back(cat.getPersonality());
        }
        
        saveProgress(); // 每次捕获后自动保存
    }
}
