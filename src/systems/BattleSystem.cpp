#include "BattleSystem.hpp"
#include <iostream>
#include <random>

BattleSystem::BattleSystem() 
    : currentState(BattleState::BATTLE_START), battleResult(BattleResult::DRAW),
      currentPlayerMeowmonIndex(0), currentEnemyMeowmonIndex(0),
      animationTimer(0.0f), animationDuration(1.0f) {
}

void BattleSystem::startBattle(std::vector<std::shared_ptr<Meowmon>> playerTeam, 
                             std::vector<std::shared_ptr<Meowmon>> enemyTeam) {
    this->playerTeam = playerTeam;
    this->enemyTeam = enemyTeam;
    currentState = BattleState::BATTLE_START;
    battleResult = BattleResult::DRAW;
    currentPlayerMeowmonIndex = 0;
    currentEnemyMeowmonIndex = 0;
    animationTimer = 0.0f;
    
    battleMessages.clear();
    battleMessages.push_back("战斗开始！");
    battleMessages.push_back("敌方派出了 " + enemyTeam[0]->getName() + "！");
    battleMessages.push_back("我方派出了 " + playerTeam[0]->getName() + "！");
}

void BattleSystem::update(float deltaTime) {
    // 更新动画计时器
    if (animationTimer > 0) {
        animationTimer -= deltaTime;
        if (animationTimer <= 0) {
            animationTimer = 0.0f;
        }
    }
    
    // 根据当前战斗状态更新
    switch (currentState) {
        case BattleState::BATTLE_START:
            handleBattleStart();
            break;
        case BattleState::PLAYER_TURN:
            handlePlayerTurn();
            break;
        case BattleState::ENEMY_TURN:
            handleEnemyTurn();
            break;
        case BattleState::PLAYER_ATTACKING:
            handlePlayerAttacking(deltaTime);
            break;
        case BattleState::ENEMY_ATTACKING:
            handleEnemyAttacking(deltaTime);
            break;
        case BattleState::BATTLE_END:
            handleBattleEnd();
            break;
    }
    
    // 更新所有Meowmon的状态
    for (auto& meowmon : playerTeam) {
        meowmon->update(deltaTime);
    }
    
    for (auto& meowmon : enemyTeam) {
        meowmon->update(deltaTime);
    }
}

void BattleSystem::draw(int screenWidth, int screenHeight) {
    // 绘制战斗背景
    DrawRectangle(0, 0, screenWidth, screenHeight, SKYBLUE);
    
    // 绘制战场区域
    DrawRectangle(0, screenHeight / 2, screenWidth, screenHeight / 2, GREEN);
    
    // 绘制敌方Meowmon
    if (!enemyTeam.empty() && enemyTeam[currentEnemyMeowmonIndex]) {
        Vector2 enemyPos = {screenWidth * 0.75f - 64, screenHeight * 0.3f - 64};
        enemyTeam[currentEnemyMeowmonIndex]->draw(enemyPos, 2.0f);
    }
    
    // 绘制玩家Meowmon
    if (!playerTeam.empty() && playerTeam[currentPlayerMeowmonIndex]) {
        Vector2 playerPos = {screenWidth * 0.25f - 64, screenHeight * 0.7f - 64};
        playerTeam[currentPlayerMeowmonIndex]->draw(playerPos, 2.0f);
    }
    
    // 绘制战斗信息
    int messageY = screenHeight - 100;
    for (const auto& message : battleMessages) {
        DrawText(message.c_str(), 10, messageY, 20, BLACK);
        messageY -= 30;
        if (messageY < 0) break;
    }
    
    // 绘制当前回合信息
    if (currentState == BattleState::PLAYER_TURN) {
        DrawText("你的回合！", screenWidth / 2 - 60, 20, 25, BLUE);
    } else if (currentState == BattleState::ENEMY_TURN) {
        DrawText("敌方回合！", screenWidth / 2 - 60, 20, 25, RED);
    }
    
    // 绘制技能选择界面
    if (currentState == BattleState::PLAYER_TURN && !playerTeam.empty()) {
        const auto& skills = playerTeam[currentPlayerMeowmonIndex]->getSkills();
        float buttonWidth = 150.0f;
        float buttonHeight = 50.0f;
        
        for (size_t i = 0; i < skills.size(); ++i) {
            float x = 10 + i * (buttonWidth + 10);
            float y = screenHeight - buttonHeight - 10;
            
            // 绘制技能按钮
            DrawRectangleRec({x, y, buttonWidth, buttonHeight}, LIGHTGRAY);
            DrawRectangleLines(x, y, buttonWidth, buttonHeight, GRAY);
            
            // 绘制技能名称
            DrawText(skills[i].name.c_str(), static_cast<int>(x + 10), static_cast<int>(y + 10), 15, BLACK);
            
            // 绘制PP
            std::string ppText = std::to_string(skills[i].pp) + "/" + std::to_string(skills[i].maxPP);
            DrawText(ppText.c_str(), static_cast<int>(x + buttonWidth - 60), static_cast<int>(y + 30), 12, GRAY);
        }
    }
    
    // 绘制战斗结果
    if (currentState == BattleState::BATTLE_END) {
        std::string resultText;
        Color resultColor;
        
        if (battleResult == BattleResult::PLAYER_WIN) {
            resultText = "你赢了！";
            resultColor = GREEN;
        } else if (battleResult == BattleResult::PLAYER_LOSE) {
            resultText = "你输了...";
            resultColor = RED;
        } else {
            resultText = "平局！";
            resultColor = YELLOW;
        }
        
        DrawText(resultText.c_str(), screenWidth / 2 - 100, screenHeight / 2 - 30, 40, resultColor);
        DrawText("按ESC键退出战斗", screenWidth / 2 - 120, screenHeight / 2 + 20, 20, BLACK);
    }
}

void BattleSystem::playerSelectSkill(int skillIndex) {
    if (currentState != BattleState::PLAYER_TURN) return;
    
    if (skillIndex >= 0 && skillIndex < playerTeam[currentPlayerMeowmonIndex]->getSkills().size()) {
        // 执行攻击
        currentState = BattleState::PLAYER_ATTACKING;
        animationTimer = animationDuration;
        
        // 记录攻击信息
        const auto& skill = playerTeam[currentPlayerMeowmonIndex]->getSkills()[skillIndex];
        battleMessages.insert(battleMessages.begin(), 
                            playerTeam[currentPlayerMeowmonIndex]->getName() + "使用了 " + skill.name + "！");
        
        // 限制消息数量
        if (battleMessages.size() > 5) {
            battleMessages.pop_back();
        }
    }
}

void BattleSystem::switchPlayerMeowmon(int index) {
    if (currentState != BattleState::PLAYER_TURN) return;
    
    if (index >= 0 && index < playerTeam.size() && playerTeam[index]->isAlive()) {
        currentPlayerMeowmonIndex = index;
        battleMessages.insert(battleMessages.begin(), 
                            "切换到 " + playerTeam[currentPlayerMeowmonIndex]->getName() + "！");
        
        // 限制消息数量
        if (battleMessages.size() > 5) {
            battleMessages.pop_back();
        }
        
        // 结束玩家回合
        currentState = BattleState::ENEMY_TURN;
    }
}

BattleSystem::BattleState BattleSystem::getBattleState() const {
    return currentState;
}

BattleSystem::BattleResult BattleSystem::getBattleResult() const {
    return battleResult;
}

bool BattleSystem::isBattleOver() const {
    return currentState == BattleState::BATTLE_END;
}

void BattleSystem::endBattle() {
    currentState = BattleState::BATTLE_END;
}

void BattleSystem::handleBattleStart() {
    // 战斗开始动画结束后，进入玩家回合
    animationTimer -= GetFrameTime();
    if (animationTimer <= 0) {
        currentState = BattleState::PLAYER_TURN;
    }
}

void BattleSystem::handlePlayerTurn() {
    // 等待玩家输入
    // 当玩家选择技能后，会切换到PLAYER_ATTACKING状态
}

void BattleSystem::handleEnemyTurn() {
    // 敌方AI选择技能
    if (!enemyTeam.empty() && !playerTeam.empty()) {
        int skillIndex = enemyChooseSkill(*enemyTeam[currentEnemyMeowmonIndex], *playerTeam[currentPlayerMeowmonIndex]);
        
        // 执行攻击
        currentState = BattleState::ENEMY_ATTACKING;
        animationTimer = animationDuration;
        
        // 记录攻击信息
        const auto& skill = enemyTeam[currentEnemyMeowmonIndex]->getSkills()[skillIndex];
        battleMessages.insert(battleMessages.begin(), 
                            enemyTeam[currentEnemyMeowmonIndex]->getName() + "使用了 " + skill.name + "！");
        
        // 限制消息数量
        if (battleMessages.size() > 5) {
            battleMessages.pop_back();
        }
    }
}

void BattleSystem::handlePlayerAttacking(float deltaTime) {
    // 攻击动画进行中
    if (animationTimer <= 0) {
        // 动画结束，执行实际攻击
        if (!playerTeam.empty() && !enemyTeam.empty()) {
            // 默认使用第一个技能攻击
            playerTeam[currentPlayerMeowmonIndex]->attack(*enemyTeam[currentEnemyMeowmonIndex], 1);
            
            // 检查敌方Meowmon是否被击败
            if (!enemyTeam[currentEnemyMeowmonIndex]->isAlive()) {
                battleMessages.insert(battleMessages.begin(), 
                                    enemyTeam[currentEnemyMeowmonIndex]->getName() + "被击败了！");
                
                // 限制消息数量
                if (battleMessages.size() > 5) {
                    battleMessages.pop_back();
                }
                
                // 检查敌方队伍是否全灭
                if (isTeamDefeated(enemyTeam)) {
                    battleResult = BattleResult::PLAYER_WIN;
                    endBattle();
                    return;
                }
                
                // 切换到下一只敌方Meowmon
                do {
                    currentEnemyMeowmonIndex = (currentEnemyMeowmonIndex + 1) % enemyTeam.size();
                } while (!enemyTeam[currentEnemyMeowmonIndex]->isAlive() && !isTeamDefeated(enemyTeam));
                
                battleMessages.insert(battleMessages.begin(), 
                                    "敌方派出了 " + enemyTeam[currentEnemyMeowmonIndex]->getName() + "！");
                
                // 限制消息数量
                if (battleMessages.size() > 5) {
                    battleMessages.pop_back();
                }
            }
        }
        
        // 攻击完成，切换到敌方回合
        currentState = BattleState::ENEMY_TURN;
    }
}

void BattleSystem::handleEnemyAttacking(float deltaTime) {
    // 攻击动画进行中
    if (animationTimer <= 0) {
        // 动画结束，执行实际攻击
        if (!enemyTeam.empty() && !playerTeam.empty()) {
            // 默认使用第一个技能攻击
            enemyTeam[currentEnemyMeowmonIndex]->attack(*playerTeam[currentPlayerMeowmonIndex], 1);
            
            // 检查玩家Meowmon是否被击败
            if (!playerTeam[currentPlayerMeowmonIndex]->isAlive()) {
                battleMessages.insert(battleMessages.begin(), 
                                    playerTeam[currentPlayerMeowmonIndex]->getName() + "被击败了！");
                
                // 限制消息数量
                if (battleMessages.size() > 5) {
                    battleMessages.pop_back();
                }
                
                // 检查玩家队伍是否全灭
                if (isTeamDefeated(playerTeam)) {
                    battleResult = BattleResult::PLAYER_LOSE;
                    endBattle();
                    return;
                }
                
                // 自动切换到下一只存活的玩家Meowmon
                do {
                    currentPlayerMeowmonIndex = (currentPlayerMeowmonIndex + 1) % playerTeam.size();
                } while (!playerTeam[currentPlayerMeowmonIndex]->isAlive() && !isTeamDefeated(playerTeam));
                
                battleMessages.insert(battleMessages.begin(), 
                                    "你派出了 " + playerTeam[currentPlayerMeowmonIndex]->getName() + "！");
                
                // 限制消息数量
                if (battleMessages.size() > 5) {
                    battleMessages.pop_back();
                }
            }
        }
        
        // 攻击完成，切换到玩家回合
        currentState = BattleState::PLAYER_TURN;
    }
}

void BattleSystem::handleBattleEnd() {
    // 战斗结束逻辑
    // 这里可以添加经验值结算、奖励等
}

int BattleSystem::enemyChooseSkill(const Meowmon& enemy, const Meowmon& player) {
    // 简单的AI决策：随机选择一个有PP的技能
    const auto& skills = enemy.getSkills();
    std::vector<int> availableSkills;
    
    for (size_t i = 0; i < skills.size(); ++i) {
        if (skills[i].pp > 0) {
            availableSkills.push_back(static_cast<int>(i));
        }
    }
    
    if (availableSkills.empty()) {
        return 0; // 所有技能都没有PP了，只能用第一个
    }
    
    // 随机选择一个技能
    return availableSkills[rand() % availableSkills.size()];
}

bool BattleSystem::isTeamDefeated(const std::vector<std::shared_ptr<Meowmon>>& team) const {
    for (const auto& meowmon : team) {
        if (meowmon->isAlive()) {
            return false;
        }
    }
    return true;
}
