#ifndef MEOWMON_HPP
#define MEOWMON_HPP

#include <string>
#include <vector>
#include <raylib.h>

// 技能类型枚举
enum class SkillType {
    NORMAL,
    FIRE,
    WATER,
    GRASS,
    ELECTRIC,
    ICE,
    FIGHTING,
    POISON,
    GROUND,
    FLYING,
    PSYCHIC,
    BUG,
    ROCK,
    GHOST,
    DRAGON,
    DARK,
    STEEL,
    FAIRY
};

// 技能结构体
struct Skill {
    std::string name;
    SkillType type;
    int power;
    int accuracy;
    int pp;
    int maxPP;
    std::string description;
};

// Meowmon类
class Meowmon {
public:
    // 构造函数
    Meowmon(const std::string& name, SkillType type, int level = 1);
    
    // 更新Meowmon状态
    void update(float deltaTime);
    
    // 绘制Meowmon
    void draw(Vector2 position, float scale = 1.0f);
    
    // 攻击另一只Meowmon
    void attack(Meowmon& target, int skillIndex);
    
    // 使用技能
    void useSkill(Meowmon& target, const Skill& skill);
    
    // 获得经验值
    void gainExperience(int exp);
    
    // 升级
    void levelUp();
    
    // 检查是否可以进化
    bool canEvolve() const;
    
    // 进化
    void evolve(const std::string& evolvedForm);
    
    // 设置/获取属性
    void setName(const std::string& name);
    std::string getName() const;
    
    void setLevel(int level);
    int getLevel() const;
    
    void setHealth(int health);
    int getHealth() const;
    
    int getMaxHealth() const;
    
    void setAttack(int attack);
    int getAttack() const;
    
    void setDefense(int defense);
    int getDefense() const;
    
    void setSpeed(int speed);
    int getSpeed() const;
    
    SkillType getType() const;
    
    // 获取技能列表
    const std::vector<Skill>& getSkills() const;
    
    // 添加技能
    void addSkill(const Skill& skill);
    
    // 检查是否存活
    bool isAlive() const;
    
    // 获取属性效果倍率
    float getTypeEffectiveness(SkillType attackerType, SkillType defenderType) const;
    
private:
    // 基本属性
    std::string name;
    SkillType type;
    int level;
    int experience;
    int nextLevelExp;
    
    // 战斗属性
    int maxHealth;
    int currentHealth;
    int attackValue;
    int defense;
    int speed;
    
    // 技能
    std::vector<Skill> skills;
    
    // 精灵图
    Texture2D sprite;
    
    // 进化信息
    std::string evolvedFormName;
    int evolutionLevel;
    
    // 初始化属性
    void initStats();
    
    // 学习新技能
    void learnNewSkill();
};

#endif // MEOWMON_HPP
