#include <raylib.h>
#include "entities/Player.hpp"
#include "entities/Cat.hpp"
#include "entities/Catnip.hpp"
#include "systems/MapLoader.hpp"
#include "core/ResourceManager.hpp"
#include "core/GameState.hpp"
#include "core/StartScreen.hpp"
#include "core/SettingsMenu.hpp"
#include "core/Meowdex.hpp"
#include "core/UIHelper.hpp"
#include <iostream>
#include <vector>
#include <memory>

int main() {
    // 初始化窗口，设置标题和大小
    InitWindow(800, 600, "Meowmon - Catnip Catcher");
    
    SetExitKey(KEY_NULL); // 禁止 ESC 键直接退出游戏

    // 状态初始化
    GameState currentState = GameState::START_SCREEN;
    
    // 初始化相机
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 400, 300 }; // 屏幕中心
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    float screenShake = 0.0f;
    
    // 初始化组件
    StartScreen startScreen;
    std::unique_ptr<Player> player = nullptr;
    std::unique_ptr<std::vector<Cat>> cats = nullptr;
    std::unique_ptr<MapLoader> mapLoader = nullptr;
    std::unique_ptr<SettingsMenu> settingsMenu = std::make_unique<SettingsMenu>();
    std::unique_ptr<Meowdex> meowdex = std::make_unique<Meowdex>();
    int caughtCount = 0;
    bool gameInitialized = false;
    bool useChinese = true; // 默认使用中文
    bool showDebug = false; // 是否显示调试信息 (F1)
    
    // 加载中文字体
    Font chineseFont = ResourceManager::getInstance().loadFont("assets/fonts/chinese_font.ttf", 20);
    bool hasFont = chineseFont.texture.id != 0;
    
    // 主游戏循环
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // 快捷键切换语言
        if (IsKeyPressed(KEY_L)) {
            useChinese = !useChinese;
        }
        
        // 切换图鉴
        if (IsKeyPressed(KEY_M)) {
            if (meowdex) meowdex->toggleVisibility();
        }

        // F1 切换调试模式
        if (IsKeyPressed(KEY_F1)) {
            showDebug = !showDebug;
        }
        
        // ESC 呼出菜单
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (settingsMenu && !settingsMenu->isMenuVisible()) {
                settingsMenu->show();
            }
        }
        
        // 全局更新设置菜单
        if (settingsMenu) settingsMenu->update(deltaTime);
        
        // 如果菜单或图鉴显示，暂停其他逻辑输入
        if ((settingsMenu && settingsMenu->isMenuVisible()) || (meowdex && meowdex->getIsVisible())) {
            // 可以在这里添加一些暂停逻辑，或者直接跳过状态更新
        } else {
            // 根据游戏状态处理不同的逻辑
            switch (currentState) {
                case GameState::START_SCREEN:
                    // 更新开始界面
                    startScreen.update(deltaTime);
                    
                    // 检查是否开始游戏
                    if (startScreen.shouldStartGame()) {
                        std::cout << "检测到开始游戏信号，切换到PLAYING状态" << std::endl;
                        currentState = GameState::PLAYING;
                        
                        // 初始化游戏对象
                        if (!gameInitialized) {
                            std::cout << "初始化游戏对象..." << std::endl;
                            // 创建地图加载器
                            mapLoader = std::make_unique<MapLoader>();
                            
                            // 尝试加载草地图 - 使用TMX格式
                            std::string mapPath = "assets/maps/grass block.tmx";
                            std::cout << "尝试加载草地图文件: " << mapPath << std::endl;
                            if (!mapLoader->loadMap(mapPath)) {
                                // 尝试备用路径
                                mapPath = "../assets/maps/grass block.tmx";
                                if (!mapLoader->loadMap(mapPath)) {
                                    std::cout << "无法加载草地图文件，使用默认设置" << std::endl;
                                } else {
                                    std::cout << "草地图加载成功 (备用路径)！" << std::endl;
                                }
                            } else {
                                std::cout << "草地图加载成功！" << std::endl;
                            }
                            
                            // 创建玩家
                            player = std::make_unique<Player>("Player1", Vector2{100.0f, 100.0f});
                            player->setSpeed(200.0f);
                            
                            // 创建猫咪 - 使用英文名字避免中文乱码
                            cats = std::make_unique<std::vector<Cat>>();
                            cats->push_back(Cat("Mimi", {200.0f, 200.0f}, CatType::PERSIAN));
                            cats->push_back(Cat("Whiskers", {400.0f, 300.0f}, CatType::SIAMESE));
                            cats->push_back(Cat("Shadow", {600.0f, 400.0f}, CatType::MAINE_COON));
                            cats->push_back(Cat("Luna", {300.0f, 500.0f}, CatType::RAGDOLL));
                            
                            gameInitialized = true;
                            caughtCount = 0;
                            std::cout << "游戏对象初始化完成" << std::endl;
                        }
                    }
                    break;
                    
                case GameState::PLAYING:
                    // 游戏主逻辑
                    if (player && cats && mapLoader) {
                        // 处理玩家输入
                        player->handleInput();
                        
                        // 更新玩家位置
                        player->update(deltaTime);
                        
                        // 获取地图尺寸
                        int mapWidth = mapLoader->getMapWidth();
                        int mapHeight = mapLoader->getMapHeight();
                        
                        // 边界检测
                        player->checkBoundaries(mapWidth, mapHeight);
                        
                        // 更新猫咪
                        for (auto& cat : *cats) {
                            // 更新猫咪状态（基于玩家和猫薄荷，传递抓到数量）
                            bool hasCatnip = player->isCatnipActive();
                            Vector2 catnipPos = player->getCatnipPosition();
                            cat.updateState(player->getPosition(), catnipPos, hasCatnip, player->getCapturedCount(), deltaTime);
                            
                            cat.update(deltaTime, player->getPosition(), catnipPos, hasCatnip, player->getCapturedCount());
                            cat.checkBoundaries(mapWidth, mapHeight);
                            
                            // 检查是否被抓到
                            if (!cat.isCaughtStatus() && cat.checkCollision(player->getRect())) {
                                if (cat.getState() == CatState::CATNIPPED) {
                                    cat.setCaught(true);
                                    caughtCount++;
                                    player->incrementCapturedCount();
                                    if (meowdex) meowdex->recordCapture(cat);
                                    screenShake = 0.5f; // 抓到时震动
                                }
                            }
                        }

                        // 更新相机
                        camera.target = player->getPosition();
                        if (screenShake > 0) {
                            camera.offset.x = 400 + GetRandomValue(-10, 10) * screenShake;
                            camera.offset.y = 300 + GetRandomValue(-10, 10) * screenShake;
                            screenShake -= deltaTime * 2.0f;
                        } else {
                            camera.offset = { 400, 300 };
                        }

                    // 动态刷新系统：如果地图上的活猫少于 4 只，尝试生成新的
                    int activeCats = 0;
                    for (const auto& c : *cats) if (!c.isCaughtStatus()) activeCats++;

                    if (activeCats < 4) {
                        float spawnX = (float)(GetRandomValue(50, mapWidth - 50));
                        float spawnY = (float)(GetRandomValue(50, mapHeight - 50));
                        
                        CatType randomType = (CatType)(GetRandomValue(0, 4));
                        const char* names[] = {"Mimi", "Whiskers", "Shadow", "Luna", "Oliver", "Leo", "Milo", "Bella"};
                        const char* randomName = names[GetRandomValue(0, 7)];
                        
                        cats->push_back(Cat(randomName, {spawnX, spawnY}, randomType));
                        std::cout << "A new cat appeared: " << randomName << " at (" << spawnX << ", " << spawnY << ")" << std::endl;
                    }

                    // 定期清理已抓获的猫咪对象，防止 vector 无限增长
                    static float cleanupTimer = 0;
                    cleanupTimer += deltaTime;
                    if (cleanupTimer > 5.0f) { // 每 5 秒清理一次
                        cats->erase(std::remove_if(cats->begin(), cats->end(), 
                            [](const Cat& c) { return c.isCaughtStatus(); }), cats->end());
                        cleanupTimer = 0;
                    }
                }
                    break;

                case GameState::PAUSED:
                case GameState::GAME_OVER:
                case GameState::SETTINGS:
                case GameState::COLLECTION:
                    // 这些状态下暂无特殊更新逻辑，或由外部组件处理
                    break;
            }
        }
        
        // --- 4. 绘制游戏内容 ---
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        switch (currentState) {
            case GameState::START_SCREEN:
                startScreen.draw();
                break;
                
            case GameState::PLAYING:
                if (gameInitialized) {
                    BeginMode2D(camera);
                    
                    // 绘制地图
                    mapLoader->draw();
                    
                    // 绘制猫咪
                    for (auto& cat : *cats) {
                        cat.draw();
                    }
                    
                    // 绘制玩家
                    player->draw();
                    
                    EndMode2D();
                    
                    // --- 5. 绘制新版 HUD (不需要相机) ---
                    // 顶栏背景
                    DrawRectangleGradientV(0, 0, 800, 60, Fade(BLACK, 0.8f), Fade(BLACK, 0.0f));
                    
                    // 捕获统计 (左侧)
                    Color caughtColor = (caughtCount >= 10) ? GOLD : YELLOW;
                    if (hasFont) {
                        DrawTextEx(chineseFont, TextFormat("🐾 %d", caughtCount), {25, 15}, 24, 1, caughtColor);
                    } else {
                        DrawText(TextFormat("🐾 %d", caughtCount), 25, 15, 20, caughtColor);
                    }

                    // 猫薄荷状态 (居中)
                    float cooldown = player->getCatnipCooldown();
                    Color nipColor = (cooldown > 0.0f) ? RED : GREEN;
                    const char* nipText = (cooldown > 0.0f) ? TextFormat("🌿 %.1fs", cooldown) : "🌿 READY";
                    if (hasFont) {
                        Vector2 nipSize = MeasureTextEx(chineseFont, nipText, 20, 1);
                        DrawTextEx(chineseFont, nipText, {400 - nipSize.x/2, 18}, 20, 1, nipColor);
                    } else {
                        DrawText(nipText, 400 - MeasureText(nipText, 20)/2, 18, 20, nipColor);
                    }

                    // 设置/菜单按钮提示 (右侧)
                    if (hasFont) {
                        DrawTextEx(chineseFont, "[ESC] MENU", {680, 18}, 16, 1, LIGHTGRAY);
                    }

                    // 只有在调试模式下才显示详细数据
                    if (showDebug) {
                        DrawRectangle(10, 70, 220, 200, Fade(BLACK, 0.5f));
                        DrawRectangleLines(10, 70, 220, 200, SKYBLUE);
                        int dy = 80;
                        DrawText(TextFormat("FPS: %i", GetFPS()), 20, dy, 15, LIME); dy += 20;
                        DrawText(TextFormat("POS: %.0f, %.0f", player->getPosition().x, player->getPosition().y), 20, dy, 15, WHITE); dy += 20;
                        DrawText(TextFormat("MAP: %dx%d", mapLoader->getMapWidth(), mapLoader->getMapHeight()), 20, dy, 15, WHITE); dy += 20;
                    }
                    
                    // 底部操作指引 (改为简洁的图标/文字)
                    DrawRectangleGradientV(0, 540, 800, 60, Fade(BLACK, 0.0f), Fade(BLACK, 0.8f));
                    const char* guide = useChinese ? "[空格] 投掷  [M] 图鉴  [WASD] 移动" : "[SPACE] Throw  [M] Dex  [WASD] Move";
                    if (hasFont) {
                        Vector2 gSize = MeasureTextEx(chineseFont, guide, 18, 1);
                        DrawTextEx(chineseFont, guide, {400 - gSize.x/2, 565}, 18, 1, Fade(WHITE, 0.8f));
                    } else {
                        DrawText(guide, 400 - MeasureText(guide, 15)/2, 565, 15, GRAY);
                    }

    // 阶段性胜利提示 (MISSION ACCOMPLISHED)
    if (caughtCount >= 10 && gameInitialized) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
        const char* victoryText = useChinese ? "恭喜！你已成为猫咪收集大师" : "MASTER COLLECTOR!";
        if (hasFont && useChinese) {
            Vector2 vSize = MeasureTextEx(chineseFont, victoryText, 40, 1);
            DrawTextEx(chineseFont, victoryText, { (800 - vSize.x) / 2, 280 }, 40, 1, GOLD);
        } else {
            int vWidth = MeasureText(victoryText, 40);
            DrawText(victoryText, (800 - vWidth) / 2, 280, 40, GOLD);
        }
        
        const char* restartText = useChinese ? "按 [M] 查看图鉴，收集进度已永久保存" : "Press [M] to view Meowdex, progress saved";
        if (hasFont) {
            Vector2 rSize = MeasureTextEx(chineseFont, restartText, 20, 1);
            DrawTextEx(chineseFont, restartText, { (800 - rSize.x) / 2, 340 }, 20, 1, WHITE);
        } else {
            DrawText(restartText, (800 - MeasureText(restartText, 20)) / 2, 340, 20, WHITE);
        }
    }
                    
                }
                break;

            case GameState::PAUSED:
            case GameState::GAME_OVER:
            case GameState::SETTINGS:
            case GameState::COLLECTION:
                // 绘制逻辑主要由外部 settingsMenu 和 meowdex 处理
                break;
        }
        
        // 绘制图鉴 (顶层)
        if (meowdex) meowdex->draw();
        
        // 全局绘制设置菜单（置顶显示）
        if (settingsMenu) settingsMenu->draw();
        
        EndDrawing();
    }
    
    // 清理资源
    player.reset();
    cats.reset();
    mapLoader.reset();
    
    ResourceManager::getInstance().unloadAll();
    CloseWindow();
    
    return 0;
}
