#include "Game.hpp"
#include <iostream>
#include "ResourceManager.hpp"

Game::Game(int width, int height, const std::string& title)
    : screenWidth(width), screenHeight(height), windowTitle(title),
      currentState(GAME_TITLE), isRunning(true), frameTime(0.0f), fps(0) {
}

Game::~Game() {
    cleanup();
}

void Game::init() {
    // 初始化raylib
    InitWindow(screenWidth, screenHeight, windowTitle.c_str());
    
    // 设置目标FPS
    SetTargetFPS(60);
    
    // 初始化音频设备
    InitAudioDevice();
    
    std::cout << "游戏初始化完成！" << std::endl;
}

void Game::run() {
    init();
    
    // 游戏主循环
    while (isRunning && !WindowShouldClose()) {
        // 计算delta时间
        frameTime = GetFrameTime();
        fps = GetFPS();
        
        // 处理输入
        handleInput();
        
        // 更新游戏逻辑
        update(frameTime);
        
        // 绘制游戏画面
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        draw();
        
        EndDrawing();
    }
    
    cleanup();
}

void Game::update(float deltaTime) {
    // 根据当前游戏状态更新
    switch (currentState) {
        case GAME_TITLE:
            handleTitleState();
            break;
        case GAME_PLAYING:
            handlePlayingState(deltaTime);
            break;
        case GAME_BATTLE:
            handleBattleState(deltaTime);
            break;
        case GAME_MENU:
            handleMenuState();
            break;
        case GAME_GAME_OVER:
            handleGameOverState();
            break;
    }
}

void Game::draw() {
    // 根据当前游戏状态绘制
    switch (currentState) {
        case GAME_TITLE:
            // Draw title screen
            DrawText("Meowmon", 280, 200, 48, RED);
            DrawText("Press ENTER to start", 230, 300, 20, DARKGRAY);
            break;
        case GAME_PLAYING:
            // Draw game world (placeholder)
            DrawText("Game Playing", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("FPS: %d", fps), 10, 40, 15, DARKGRAY);
            break;
        case GAME_BATTLE:
            // Draw battle system (placeholder)
            DrawText("Battle System", screenWidth / 2 - 80, screenHeight / 2, 20, RED);
            break;
        case GAME_MENU:
            DrawText("游戏菜单", screenWidth / 2 - 50, 10, 20, BLUE);
            break;
        case GAME_GAME_OVER:
            // Draw game over screen
            DrawText("Game Over", 300, 250, 36, RED);
            DrawText("Press R to restart", 260, 320, 20, DARKGRAY);
            break;
    }
}

void Game::handleInput() {
    // 全局输入处理
    if (IsKeyPressed(KEY_ESCAPE)) {
        isRunning = false;
    }
    
    // 根据当前状态处理输入
    switch (currentState) {
        case GAME_TITLE:
            if (IsKeyPressed(KEY_SPACE)) {
                setState(GAME_PLAYING);
            }
            break;
        case GAME_PLAYING:
            if (IsKeyPressed(KEY_M)) {
                setState(GAME_MENU);
            } else if (IsKeyPressed(KEY_B)) {
                setState(GAME_BATTLE);
            }
            break;
        case GAME_BATTLE:
            if (IsKeyPressed(KEY_ESCAPE)) {
                setState(GAME_PLAYING);
            }
            break;
        case GAME_MENU:
            if (IsKeyPressed(KEY_ESCAPE)) {
                setState(GAME_PLAYING);
            }
            break;
        case GAME_GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                setState(GAME_TITLE);
            }
            break;
    }
}

void Game::setState(GameState state) {
    currentState = state;
}

GameState Game::getState() const {
    return currentState;
}

int Game::getWidth() const {
    return screenWidth;
}

int Game::getHeight() const {
    return screenHeight;
}

void Game::cleanup() {
    // 清理资源管理器
    ResourceManager::getInstance().unloadAll();
    
    // 关闭音频设备
    CloseAudioDevice();
    
    // 关闭窗口
    CloseWindow();
    
    std::cout << "游戏清理完成！" << std::endl;
}

// 状态处理函数
void Game::handleTitleState() {
    // 标题画面逻辑
}

void Game::handlePlayingState(float deltaTime) {
    // 游戏进行中逻辑
}

void Game::handleBattleState(float deltaTime) {
    // 战斗模式逻辑
}

void Game::handleMenuState() {
    // 菜单逻辑
}

void Game::handleGameOverState() {
    // 游戏结束逻辑
}
