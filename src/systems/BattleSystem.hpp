#ifndef BATTLESYSTEM_HPP
#define BATTLESYSTEM_HPP

#include <vector>
#include <memory>
#include "entities/Meowmon.hpp"

class BattleSystem {
public:
    // 战斗状态枚举
enum BattleState {
    BATTLE_START,
    PLAYER_TURN,
    ENEMY_TURN,
    PLAYER_ATTACKING,
    ENEMY_ATTACKING,
    BATTLE_END
};
    
    // 战斗结果枚举
enum BattleResult {
    PLAYER_WIN,
    PLAYER_LOSE,
    DRAW
};
    
    // 构造函数
    BattleSystem();
    
    // 开始战斗
    void startBattle(std::vector<std::shared_ptr<Meowmon>> playerTeam, 
                    std::vector<std::shared_ptr<Meowmon>> enemyTeam);
    
    // 更新战斗系统
    void update(float deltaTime);
    
    // 绘制战斗界面
    void draw(int screenWidth, int screenHeight);
    
    // 玩家选择技能
    void playerSelectSkill(int skillIndex);
    
    // 切换玩家当前使用的Meowmon
    void switchPlayerMeowmon(int index);
    
    // 获取当前战斗状态
    BattleState getBattleState() const;
    
    // 获取战斗结果
    BattleResult getBattleResult() const;
    
    // 检查战斗是否结束
    bool isBattleOver() const;
    
    // 结束战斗
    void endBattle();
    
private:
    // 战斗逻辑
    void handleBattleStart();
    void handlePlayerTurn();
    void handleEnemyTurn();
    void handlePlayerAttacking(float deltaTime);
    void handleEnemyAttacking(float deltaTime);
    void handleBattleEnd();
    
    // 敌方AI决策
    int enemyChooseSkill(const Meowmon& enemy, const Meowmon& player);
    
    // 战斗属性
    BattleState currentState;
    BattleResult battleResult;
    
    std::vector<std::shared_ptr<Meowmon>> playerTeam;
    std::vector<std::shared_ptr<Meowmon>> enemyTeam;
    
    int currentPlayerMeowmonIndex;
    int currentEnemyMeowmonIndex;
    
    // 战斗动画相关
    float animationTimer;
    float animationDuration;
    
    // 战斗信息文本
    std::vector<std::string> battleMessages;
    
    // 检查队伍是否全灭
    bool isTeamDefeated(const std::vector<std::shared_ptr<Meowmon>>& team) const;
};

#endif // BATTLESYSTEM_HPP
