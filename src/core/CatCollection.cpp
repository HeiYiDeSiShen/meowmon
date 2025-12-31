#include "CatCollection.hpp"
#include "UIHelper.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <cmath>

CatCollection::CatCollection() : visible(false), isObserving(false), selectedIndex(0), scrollOffset(0.0f), observationTimer(0.0f) {
    initCollection();
}

CatCollection::~CatCollection() {}

void CatCollection::initCollection() {
    items.clear();
    items.push_back({CatType::PERSIAN, "波斯猫", false, 0});
    items.push_back({CatType::SIAMESE, "暹罗猫", false, 0});
    items.push_back({CatType::MAINE_COON, "缅因猫", false, 0});
    items.push_back({CatType::RAGDOLL, "布偶猫", false, 0});
    items.push_back({CatType::BENGAL, "孟加拉猫", false, 0});
}

void CatCollection::addCat(CatType type, const std::string& name) {
    for (auto& item : items) {
        if (item.type == type) {
            item.discovered = true;
            item.count++;
            break;
        }
    }
}

void CatCollection::update(float deltaTime) {
    if (!visible) return;

    if (isObserving) {
        observationTimer += deltaTime;
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            isObserving = false;
        }
        return;
    }

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DOWN)) {
        selectedIndex = (selectedIndex + 1) % items.size();
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP)) {
        selectedIndex = (selectedIndex - 1 + items.size()) % items.size();
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (items[selectedIndex].discovered) {
            isObserving = true;
            observationTimer = 0.0f;
        }
    }

    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_ESCAPE)) {
        visible = false;
    }
}

void CatCollection::draw(bool useChinese) {
    if (!visible) return;

    // 预加载字体以提高性能并解决乱码
    Font chineseFont = ResourceManager::getInstance().getFont("assets/fonts/chinese_font.ttf");
    bool hasFont = chineseFont.texture.id != 0;

    // 绘制半透明背景
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
    
    if (isObserving) {
        drawObservationView(items[selectedIndex], useChinese);
        return;
    }

    // 绘制标题
    const char* title = useChinese ? "猫咪图鉴" : "Cat Collection";
    const char* hint = useChinese ? "A/D: 切换 | 空格: 查看 | C: 关闭" : "A/D: Switch | SPACE: Observe | C: Close";

    if (hasFont) {
        Vector2 titlePos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, title, 40, 2).x / 2, 50 };
        Vector2 hintPos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, hint, 20, 1).x / 2, 100 };
        DrawTextEx(chineseFont, title, titlePos, 40, 2, WHITE);
        DrawTextEx(chineseFont, hint, hintPos, 20, 1, LIGHTGRAY);
    } else {
        UIHelper::DrawTextCentered(title, 50, 40, WHITE);
        UIHelper::DrawTextCentered(hint, 100, 20, LIGHTGRAY);
    }

    float startX = (GetScreenWidth() - (items.size() * 140 - 20)) / 2.0f;
    float cardWidth = 120;
    float spacing = 20;
    
    for (int i = 0; i < items.size(); i++) {
        drawCard(items[i], startX + i * (cardWidth + spacing), 220, i == selectedIndex, useChinese);
    }

    // 绘制详情概要
    if (selectedIndex >= 0 && selectedIndex < items.size()) {
        const auto& item = items[selectedIndex];
        float detailY = 420;
        
        if (item.discovered) {
            if (hasFont) {
                Vector2 namePos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, item.name.c_str(), 32, 2).x / 2, detailY };
                std::string countStr = "已捕获: " + std::to_string(item.count);
                Vector2 countPos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, countStr.c_str(), 18, 1).x / 2, detailY + 45 };
                const char* obsHint = "按 [空格] 查看详细信息";
                Vector2 obsPos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, obsHint, 16, 1).x / 2, detailY + 80 };
                
                DrawTextEx(chineseFont, item.name.c_str(), namePos, 32, 2, YELLOW);
                DrawTextEx(chineseFont, countStr.c_str(), countPos, 18, 1, WHITE);
                DrawTextEx(chineseFont, obsHint, obsPos, 16, 1, SKYBLUE);
            } else {
                UIHelper::DrawTextCentered(item.name.c_str(), detailY, 32, YELLOW);
                UIHelper::DrawTextCentered(("Captured: " + std::to_string(item.count)).c_str(), detailY + 45, 18, WHITE);
                UIHelper::DrawTextCentered("Press [SPACE] for Detail View", detailY + 80, 16, SKYBLUE);
            }
        } else {
            if (hasFont) {
                const char* unknown = "???";
                const char* unknownHint = "尚未发现此猫咪";
                Vector2 namePos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, unknown, 32, 2).x / 2, detailY };
                Vector2 hintPos = { (float)GetScreenWidth()/2 - MeasureTextEx(chineseFont, unknownHint, 18, 1).x / 2, detailY + 45 };
                DrawTextEx(chineseFont, unknown, namePos, 32, 2, DARKGRAY);
                DrawTextEx(chineseFont, unknownHint, hintPos, 18, 1, GRAY);
            } else {
                UIHelper::DrawTextCentered("???", detailY, 32, DARKGRAY);
                UIHelper::DrawTextCentered("Cat not discovered yet", detailY + 45, 18, GRAY);
            }
        }
    }
}

void CatCollection::drawObservationView(const CollectionItem& item, bool useChinese) {
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;

    // 绘制复古像素风格背景
    DrawRectangle(centerX - 350, centerY - 250, 700, 500, Color{20, 20, 25, 255});
    DrawRectangleLinesEx((Rectangle){centerX - 350, centerY - 250, 700, 500}, 4, Color{60, 60, 70, 255});
    
    // 绘制像素网格装饰
    for(int i = 0; i < 700; i += 40) DrawLine(centerX - 350 + i, centerY - 250, centerX - 350 + i, centerY + 250, Color{30, 30, 40, 255});
    for(int i = 0; i < 500; i += 40) DrawLine(centerX - 350, centerY - 250 + i, centerX + 350, centerY - 250 + i, Color{30, 30, 40, 255});

    // 设置不同品种的特征颜色和细节
    Color primary = GRAY;
    Color secondary = DARKGRAY;
    Color accent = BLACK;
    Color eye = SKYBLUE;

    if (item.type == CatType::PERSIAN) {
        primary = Color{245, 240, 230, 255}; // Cream
        secondary = Color{220, 210, 190, 255}; // Shadow
        eye = Color{240, 230, 140, 255}; // #F0E68C
        accent = WHITE; // Highlight
    } else if (item.type == CatType::SIAMESE) {
        primary = Color{235, 220, 200, 255}; // 浅米色
        secondary = Color{80, 60, 50, 255};  // 深褐色重点色
        eye = Color{50, 150, 255, 255}; // 蓝眼睛
    } else if (item.type == CatType::MAINE_COON) {
        primary = Color{100, 95, 90, 255};  // 灰色虎斑
        secondary = Color{60, 55, 50, 255};
        accent = Color{40, 35, 30, 255};
        eye = Color{150, 200, 50, 255}; // 绿眼睛
    } else if (item.type == CatType::RAGDOLL) {
        primary = WHITE;
        secondary = Color{200, 210, 230, 255}; // 淡蓝色调
        eye = Color{0, 120, 255, 255}; // 深蓝眼睛
    } else if (item.type == CatType::BENGAL) {
        primary = Color{210, 160, 100, 255}; // 橘褐色
        secondary = Color{120, 80, 40, 255};  // 深色豹纹
        eye = Color{100, 180, 50, 255}; // 绿眼睛
    }

    // 基础猫咪形状 (16x16 矩阵)
    int catMatrix[16][16] = {0};
    
    if (item.type == CatType::PERSIAN) {
        // 波斯猫：圆胖蓬松，扁脸，小圆耳
        // 身体 (圆润)
        for(int y=7; y<=13; y++) {
            for(int x=4; x<=12; x++) {
                if ((y == 7 || y == 13) && (x == 4 || x == 12)) continue;
                catMatrix[y][x] = 1;
            }
        }
        // 侧向蓬松长毛
        catMatrix[9][3] = 1; catMatrix[10][3] = 1; catMatrix[11][3] = 1;
        catMatrix[9][13] = 1; catMatrix[10][13] = 1; catMatrix[11][13] = 1;

        // 头部 (下沉且圆)
        for(int y=5; y<=8; y++) {
            for(int x=5; x<=11; x++) {
                if (y == 5 && (x == 5 || x == 11)) continue;
                catMatrix[y][x] = 1;
            }
        }

        // 小圆耳 (3x3，贴头)
        for(int y=4; y<=5; y++) {
            for(int x=5; x<=7; x++) catMatrix[y][x] = 1;
            for(int x=9; x<=11; x++) catMatrix[y][x] = 1;
        }

        // 2x2 圆形眼睛带高光
        catMatrix[6][6] = 4; catMatrix[6][7] = 4;
        catMatrix[7][6] = 4; catMatrix[7][7] = 4;
        catMatrix[6][6] = 3; // 高光

        catMatrix[6][9] = 4; catMatrix[6][10] = 4;
        catMatrix[7][9] = 4; catMatrix[7][10] = 4;
        catMatrix[6][9] = 3; // 高光

        // 扁鼻子
        catMatrix[7][8] = 2;

        // 短粗蓬松尾巴
        for(int y=10; y<=12; y++) {
            for(int x=13; x<=15; x++) catMatrix[y][x] = 1;
        }
        catMatrix[9][14] = 1;
    } else {
        // 默认基础形状
        // 绘制身体 (6,5) 到 (13,11)
        for(int i=6; i<=13; i++) {
            for(int j=5; j<=11; j++) catMatrix[i][j] = 1;
        }
        
        // 绘制头部 (4,6) 到 (8,10)
        for(int i=4; i<=8; i++) {
            for(int j=6; j<=10; j++) catMatrix[i][j] = 1;
        }

        // 耳朵
        catMatrix[3][6] = 1; catMatrix[3][10] = 1;

        // 眼睛
        catMatrix[5][7] = 4; catMatrix[5][9] = 4;

        // 尾巴
        catMatrix[11][12] = 1; catMatrix[10][13] = 1; catMatrix[9][14] = 1;
    }

    // 特殊特征处理
    if (item.type == CatType::SIAMESE) {
        // 暹罗猫：深色面罩、耳朵、爪子、尾巴
        catMatrix[5][8] = 2; catMatrix[6][8] = 2; // 面罩
        catMatrix[3][6] = 2; catMatrix[3][10] = 2; // 耳朵
        catMatrix[13][5] = 2; catMatrix[13][11] = 2; // 爪子
        catMatrix[11][12] = 2; catMatrix[10][13] = 2; catMatrix[9][14] = 2; // 尾巴
    } else if (item.type == CatType::BENGAL) {
        // 孟加拉猫：豹纹斑点
        catMatrix[7][7] = 3; catMatrix[10][6] = 3; catMatrix[12][8] = 3;
        catMatrix[8][10] = 3; catMatrix[11][10] = 3;
    } else if (item.type == CatType::MAINE_COON) {
        // 缅因猫：体型更大，耳朵带簇毛，大尾巴
        catMatrix[2][6] = 1; catMatrix[2][10] = 1; // 耳朵簇毛
        for(int i=8; i<=12; i++) {
            catMatrix[i][12] = 1; catMatrix[i][13] = 1; catMatrix[i][14] = 1; // 蓬松大尾巴
        }
    } else if (item.type == CatType::RAGDOLL) {
        // 布偶猫：面部倒V字花纹，四肢重点色
        catMatrix[5][8] = 2; catMatrix[6][7] = 2; catMatrix[6][9] = 2; // 倒V
        catMatrix[13][5] = 2; catMatrix[13][11] = 2; // 重点色爪子
    }

    // 渲染矩阵
    auto drawPixel = [&](int mx, int my, Color color) {
        DrawRectangle(centerX - 120 + mx * 15, centerY - 150 + my * 15, 14, 14, color);
    };

    for(int y=0; y<16; y++) {
        for(int x=0; x<16; x++) {
            int val = catMatrix[y][x];
            if(val == 1) drawPixel(x, y, primary);
            else if(val == 2) drawPixel(x, y, secondary);
            else if(val == 3) drawPixel(x, y, accent);
            else if(val == 4) drawPixel(x, y, eye);
        }
    }

    // 获取中文字体
    Font chineseFont = ResourceManager::getInstance().getFont("assets/fonts/chinese_font.ttf");
    bool hasFont = chineseFont.texture.id != 0;

    // 绘制名字 (使用 DrawTextEx 支持中文)
    std::string displayName = item.name;
    if (!useChinese) {
        if (item.type == CatType::PERSIAN) displayName = "Persian Cat";
        else if (item.type == CatType::SIAMESE) displayName = "Siamese Cat";
        else if (item.type == CatType::MAINE_COON) displayName = "Maine Coon";
        else if (item.type == CatType::RAGDOLL) displayName = "Ragdoll Cat";
        else if (item.type == CatType::BENGAL) displayName = "Bengal Cat";
    }

    Vector2 namePos = { (float)centerX - MeasureTextEx(hasFont ? chineseFont : GetFontDefault(), displayName.c_str(), 40, 2).x / 2, (float)centerY + 120 };
    if (hasFont) {
        DrawTextEx(chineseFont, displayName.c_str(), namePos, 40, 2, YELLOW);
    } else {
        DrawText(displayName.c_str(), (int)namePos.x, (int)namePos.y, 40, YELLOW);
    }
    
    const char* desc = "";
    const char* stats = "";
    const char* backHint = useChinese ? "按 [ESC] 返回列表" : "Press [ESC] to return";

    if (item.type == CatType::PERSIAN) {
        desc = useChinese ? "波斯猫" : "Persian: Noble cat with long fluffy fur and a calm personality.";
        stats = useChinese ? "性格: 温顺 | 稀有度: *** | 速度: 较慢" : "Temper: Gentle | Rarity: *** | Speed: Slow";
    } else if (item.type == CatType::SIAMESE) {
        desc = useChinese ? "暹罗猫" : "Siamese: Short-haired cat from Thailand with unique points.";
        stats = useChinese ? "性格: 好奇 | 稀有度: *** | 速度: 极快" : "Temper: Curious | Rarity: *** | Speed: Fast";
    } else if (item.type == CatType::MAINE_COON) {
        desc = useChinese ? "缅因猫" : "Maine Coon: Large fluffy cat known as the 'Gentle Giant'.";
        stats = useChinese ? "性格: 友善 | 稀有度: **** | 速度: 中等" : "Temper: Friendly | Rarity: **** | Speed: Medium";
    } else if (item.type == CatType::RAGDOLL) {
        desc = useChinese ? "布偶猫" : "Ragdoll: Extremely docile cat that goes limp when picked up.";
        stats = useChinese ? "性格: 慵懒 | 稀有度: **** | 速度: 较慢" : "Temper: Lazy | Rarity: **** | Speed: Slow";
    } else if (item.type == CatType::BENGAL) {
        desc = useChinese ? "孟加拉猫" : "Bengal: Active cat with a beautiful wild leopard pattern.";
        stats = useChinese ? "性格: 活跃 | 稀有度: ***** | 速度: 极快" : "Temper: Active | Rarity: ***** | Speed: Very Fast";
    }

    Vector2 descPos = { (float)centerX - MeasureTextEx(hasFont ? chineseFont : GetFontDefault(), desc, 22, 1).x / 2, (float)centerY + 180 };
    Vector2 statsPos = { (float)centerX - MeasureTextEx(hasFont ? chineseFont : GetFontDefault(), stats, 20, 1).x / 2, (float)centerY + 215 };

    if (hasFont) {
        DrawTextEx(chineseFont, desc, descPos, 22, 1, WHITE);
        DrawTextEx(chineseFont, stats, statsPos, 20, 1, GRAY);
        
        Vector2 hintPos = { (float)centerX - MeasureTextEx(chineseFont, backHint, 18, 1).x / 2, (float)GetScreenHeight() - 40 };
        DrawTextEx(chineseFont, backHint, hintPos, 18, 1, LIGHTGRAY);
    } else {
        DrawText(desc, (int)descPos.x, (int)descPos.y, 22, WHITE);
        DrawText(stats, (int)statsPos.x, (int)statsPos.y, 20, GRAY);
        DrawText(backHint, centerX - MeasureText(backHint, 18)/2, GetScreenHeight() - 40, 18, LIGHTGRAY);
    }
}

void CatCollection::drawCard(const CollectionItem& item, float x, float y, bool selected, bool useChinese) {
    float width = 120;
    float height = 150;
    
    Rectangle cardRec = {x, y, width, height};
    
    // 绘制阴影
    if (selected) {
        DrawRectangleRounded((Rectangle){x + 4, y + 4, width, height}, 0.1f, 8, Fade(BLACK, 0.4f));
    }

    // 绘制边框
    DrawRectangleRounded(cardRec, 0.1f, 8, selected ? Color{45, 55, 72, 255} : Color{31, 41, 55, 255});
    DrawRectangleRoundedLines(cardRec, 0.1f, 8, selected ? YELLOW : GRAY);
    
    // 获取字体
    Font chineseFont = ResourceManager::getInstance().getFont("assets/fonts/chinese_font.ttf");
    bool hasFont = chineseFont.texture.id != 0;

    if (item.discovered) {
        // 绘制猫咪预览（像素小图标）
        Color primary = GRAY;
        if (item.type == CatType::PERSIAN) primary = Color{245, 240, 230, 255};
        else if (item.type == CatType::SIAMESE) primary = Color{235, 220, 200, 255};
        else if (item.type == CatType::MAINE_COON) primary = Color{100, 95, 90, 255};
        else if (item.type == CatType::RAGDOLL) primary = WHITE;
        else if (item.type == CatType::BENGAL) primary = Color{220, 150, 60, 255};
        
        float pulse = selected ? sin(GetTime() * 5.0f) * 2.0f : 0;
        float px = 4.0f; // 小图标像素大小
        float startIconX = x + width/2 - px * 4;
        float startIconY = y + height/2 - 20 + pulse;

        // 简易 8x8 像素猫头
        for(int py=0; py<6; py++) {
            for(int px_i=0; px_i<8; px_i++) {
                if ((py >= 1 && py <= 4 && px_i >= 1 && px_i <= 6) || // 脸
                    (py == 0 && (px_i == 1 || px_i == 6))) { // 耳
                    DrawRectangle(startIconX + px_i * px, startIconY + py * px, px - 1, px - 1, primary);
                }
            }
        }
        
        // 眼睛
        DrawRectangle(startIconX + 2 * px, startIconY + 2 * px, px - 1, px - 1, BLACK);
        DrawRectangle(startIconX + 5 * px, startIconY + 2 * px, px - 1, px - 1, BLACK);
        
        // 绘制名字
        std::string displayName = item.name;
        if (!useChinese) {
            if (item.type == CatType::PERSIAN) displayName = "Persian";
            else if (item.type == CatType::SIAMESE) displayName = "Siamese";
            else if (item.type == CatType::MAINE_COON) displayName = "Maine";
            else if (item.type == CatType::RAGDOLL) displayName = "Ragdoll";
            else if (item.type == CatType::BENGAL) displayName = "Bengal";
        }
        
        if (hasFont) {
            Vector2 nameSize = MeasureTextEx(chineseFont, displayName.c_str(), 16, 1);
            Vector2 namePos = { x + width/2 - nameSize.x/2, y + height - 30 };
            DrawTextEx(chineseFont, displayName.c_str(), namePos, 16, 1, selected ? WHITE : LIGHTGRAY);
        } else {
            int nameWidth = MeasureText(displayName.c_str(), 16);
            DrawText(displayName.c_str(), x + width/2 - nameWidth/2, y + height - 30, 16, selected ? WHITE : LIGHTGRAY);
        }
    } else {
        DrawText("?", x + width/2 - 10, y + height/2 - 20, 40, DARKGRAY);
        
        const char* unknown = useChinese ? "未解锁" : "Unknown";
        
        if (hasFont) {
            Vector2 unknownSize = MeasureTextEx(chineseFont, unknown, 16, 1);
            Vector2 unknownPos = { x + width/2 - unknownSize.x/2, y + height - 30 };
            DrawTextEx(chineseFont, unknown, unknownPos, 16, 1, Color{75, 85, 99, 255});
        } else {
            int unknownWidth = MeasureText(unknown, 16);
            DrawText(unknown, x + width/2 - unknownWidth/2, y + height - 30, 16, Color{75, 85, 99, 255});
        }
    }
}
