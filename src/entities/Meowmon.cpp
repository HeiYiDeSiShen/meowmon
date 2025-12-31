#include "Meowmon.hpp"
#include <random>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

Meowmon::Meowmon(const std::string& name, SkillType type, int level) 
    : name(name), type(type), level(level), experience(0), 
      nextLevelExp(level * 100), evolvedFormName(""), evolutionLevel(0) {
    initStats();
    
    // 尝试加载精灵图（简化版，直接使用raylib的加载函数）
    std::string spritePath = "assets/sprites/" + name + ".png";
    sprite = LoadTexture(spritePath.c_str());
    
    // 检查是否加载成功，如果失败则创建一个简单的矩形作为占位符
    if (sprite.id == 0) {
        Color typeColor = WHITE;
        if (type == SkillType::FIRE) typeColor = RED;
        else if (type == SkillType::WATER) typeColor = BLUE;
        else if (type == SkillType::GRASS) typeColor = GREEN;
        else if (type == SkillType::ELECTRIC) typeColor = YELLOW;
        else typeColor = GRAY;
        
        Image image = GenImageColor(64, 64, typeColor);
        sprite = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    
    // 初始化技能
    skills.push_back({"抓", SkillType::NORMAL, 40, 100, 35, 35, "用爪子抓敌人"});
    
    // 根据类型添加初始技能
    if (type == SkillType::FIRE) {
        skills.push_back({"火花", SkillType::FIRE, 40, 100, 25, 25, "喷出火焰攻击敌人"});
    } else if (type == SkillType::WATER) {
        skills.push_back({"水枪", SkillType::WATER, 40, 100, 25, 25, "喷射水枪攻击敌人"});
    } else if (type == SkillType::GRASS) {
        skills.push_back({"藤鞭", SkillType::GRASS, 45, 100, 25, 25, "用藤蔓鞭打敌人"});
    } else if (type == SkillType::ELECTRIC) {
        skills.push_back({"电击", SkillType::ELECTRIC, 40, 100, 30, 30, "释放电击攻击敌人"});
    } else {
        skills.push_back({"撞击", SkillType::NORMAL, 35, 95, 35, 35, "撞击敌人"});
    }
    
    // 设置进化信息（示例）
    if (name == "小火猫") {
        evolvedFormName = "火炎猫";
        evolutionLevel = 16;
    } else if (name == "水喵") {
        evolvedFormName = "巨浪猫";
        evolutionLevel = 16;
    } else if (name == "草猫") {
        evolvedFormName = "叶灵猫";
        evolutionLevel = 16;
    }
}

void Meowmon::update(float deltaTime) {
    // 这里可以添加状态效果更新、动画等
}

void Meowmon::draw(Vector2 position, float scale) {
    // 绘制Meowmon
    float width = static_cast<float>(sprite.width) * scale;
    float height = static_cast<float>(sprite.height) * scale;
    DrawTextureEx(sprite, position, 0.0f, scale, WHITE);
    
    // 绘制名称和等级
    std::string info = name + " Lv." + std::to_string(level);
    DrawText(info.c_str(), static_cast<int>(position.x), static_cast<int>(position.y - 20), 15, BLACK);
    
    // 绘制血条
    float healthBarWidth = width * 0.8f;
    float healthBarHeight = 8.0f;
    Vector2 healthBarPos = {position.x + (width - healthBarWidth) / 2, position.y - 40};
    
    // 背景
    DrawRectangleRec({healthBarPos.x, healthBarPos.y, healthBarWidth, healthBarHeight}, GRAY);
    
    // 当前血量
    float currentHealthWidth = (static_cast<float>(currentHealth) / maxHealth) * healthBarWidth;
    DrawRectangleRec({healthBarPos.x, healthBarPos.y, currentHealthWidth, healthBarHeight}, RED);
    
    // 血量文本
    std::string healthText = std::to_string(currentHealth) + "/" + std::to_string(maxHealth);
    DrawText(healthText.c_str(), static_cast<int>(healthBarPos.x + healthBarWidth / 2 - MeasureText(healthText.c_str(), 10) / 2), 
             static_cast<int>(healthBarPos.y - 15), 10, BLACK);
}

void Meowmon::attack(Meowmon& target, int skillIndex) {
    if (skillIndex >= 0 && skillIndex < skills.size()) {
        useSkill(target, skills[skillIndex]);
    }
}

void Meowmon::useSkill(Meowmon& target, const Skill& skill) {
    // 检查PP是否足够
    if (skill.pp <= 0) {
        std::cout << name << "的" << skill.name << "没有PP了！" << std::endl;
        return;
    }
    
    // 计算伤害
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> accuracyDist(0, 99);
    
    // 检查命中率
    if (accuracyDist(gen) >= skill.accuracy) {
        std::cout << name << "的" << skill.name << "没有命中！" << std::endl;
        return;
    }
    
    // 计算伤害（使用成员函数版本的getTypeEffectiveness）
    float typeEffectiveness = this->getTypeEffectiveness(skill.type, target.getType());
    int damage = static_cast<int>((((2 * level / 5 + 2) * skill.power * attackValue / target.getDefense()) / 50) + 2);
    
    // 随机伤害波动 (85% - 100%)
    std::uniform_int_distribution<> damageDist(85, 100);
    damage = (damage * damageDist(gen)) / 100;
    
    // 应用属性效果
    damage = static_cast<int>(damage * typeEffectiveness);
    
    // 最小伤害为1
    if (damage < 1) damage = 1;
    
    // 对目标造成伤害
    target.setHealth(target.getHealth() - damage);
    
    // 显示战斗信息
    std::cout << name << "使用了" << skill.name << "！" << std::endl;
    std::cout << "对" << target.getName() << "造成了" << damage << "点伤害！" << std::endl;
    
    if (typeEffectiveness > 1) {
        std::cout << "效果拔群！" << std::endl;
    } else if (typeEffectiveness < 1) {
        std::cout << "效果不佳..." << std::endl;
    }
    
    // 减少PP
    Skill& usedSkill = const_cast<Skill&>(skill);
    usedSkill.pp--;
}

void Meowmon::gainExperience(int exp) {
    experience += exp;
    std::cout << name << "获得了" << exp << "点经验值！" << std::endl;
    
    // 检查是否升级
    while (experience >= nextLevelExp) {
        experience -= nextLevelExp;
        levelUp();
    }
}

void Meowmon::levelUp() {
    level++;
    std::cout << name << "升到了" << level << "级！" << std::endl;
    
    // 提升属性
    maxHealth += 10 + (rand() % 5);
    currentHealth = maxHealth;
    attackValue += 2 + (rand() % 3);
    defense += 2 + (rand() % 3);
    speed += 1 + (rand() % 2);
    
    // 学习新技能
    learnNewSkill();
    
    // 检查进化
    if (canEvolve()) {
        evolve(evolvedFormName);
    }
    
    // 计算下一级所需经验
    nextLevelExp = static_cast<int>(nextLevelExp * 1.2f);
}

void Meowmon::learnNewSkill() {
    // 这里可以根据等级学习新技能
    if (level == 5 && skills.size() < 4) {
        skills.push_back({"吼叫", SkillType::NORMAL, 0, 100, 20, 20, "让敌人害怕并逃跑"});
        std::cout << name << "学会了新技能：吼叫！" << std::endl;
    } else if (level == 10 && skills.size() < 4) {
        skills.push_back({"疯狂乱抓", SkillType::NORMAL, 18, 80, 15, 15, "连续攻击敌人"});
        std::cout << name << "学会了新技能：疯狂乱抓！" << std::endl;
    }
}

bool Meowmon::canEvolve() const {
    return !evolvedFormName.empty() && level >= evolutionLevel;
}

void Meowmon::evolve(const std::string& evolvedForm) {
    if (canEvolve()) {
        name = evolvedForm;
        std::cout << name << "进化了！" << std::endl;
        
        // 大幅提升属性
        maxHealth += 20;
        currentHealth = maxHealth;
        attackValue += 5;
        defense += 5;
        speed += 3;
        
        // 重置进化信息
        evolvedFormName = "";
        evolutionLevel = 0;
        
        // 尝试加载进化后的精灵图
    std::string spritePath = "assets/sprites/" + name + ".png";
    Texture2D newSprite = LoadTexture(spritePath.c_str());
    if (newSprite.id != 0) {
        UnloadTexture(sprite);
        sprite = newSprite;
    } else {
        std::cout << "无法加载进化后的精灵图！" << std::endl;
        UnloadTexture(newSprite);
    }
    }
}

void Meowmon::setName(const std::string& name) {
    this->name = name;
}

std::string Meowmon::getName() const {
    return name;
}

void Meowmon::setLevel(int level) {
    this->level = level;
    initStats();
}

int Meowmon::getLevel() const {
    return level;
}

void Meowmon::setHealth(int health) {
    currentHealth = health;
    if (currentHealth > maxHealth) currentHealth = maxHealth;
    if (currentHealth < 0) currentHealth = 0;
}

int Meowmon::getHealth() const {
    return currentHealth;
}

int Meowmon::getMaxHealth() const {
    return maxHealth;
}

void Meowmon::setAttack(int attack) {
    this->attackValue = attack;
}

int Meowmon::getAttack() const {
    return attackValue;
}

void Meowmon::setDefense(int defense) {
    this->defense = defense;
}

int Meowmon::getDefense() const {
    return defense;
}

void Meowmon::setSpeed(int speed) {
    this->speed = speed;
}

int Meowmon::getSpeed() const {
    return speed;
}

SkillType Meowmon::getType() const {
    return type;
}

const std::vector<Skill>& Meowmon::getSkills() const {
    return skills;
}

float Meowmon::getTypeEffectiveness(SkillType attackerType, SkillType defenderType) const {
    // 创建属性相克表
    static std::unordered_map<SkillType, std::unordered_map<SkillType, float>> typeChart = {
        {SkillType::FIRE, {
            {SkillType::GRASS, 2.0f},
            {SkillType::WATER, 0.5f},
            {SkillType::FIRE, 0.5f}
        }},
        {SkillType::WATER, {
            {SkillType::FIRE, 2.0f},
            {SkillType::GRASS, 0.5f},
            {SkillType::WATER, 0.5f}
        }},
        {SkillType::GRASS, {
            {SkillType::WATER, 2.0f},
            {SkillType::FIRE, 0.5f},
            {SkillType::GRASS, 0.5f}
        }},
        {SkillType::ELECTRIC, {
            {SkillType::WATER, 2.0f},
            {SkillType::ELECTRIC, 0.5f}
        }},
        {SkillType::NORMAL, {
            {SkillType::NORMAL, 1.0f} // 对所有类型都是1倍伤害
        }}
    };
    
    // 检查攻击类型是否在表中
    if (typeChart.find(attackerType) != typeChart.end()) {
        // 检查防御类型是否在攻击类型的映射中
        const auto& defenderMap = typeChart.at(attackerType);
        if (defenderMap.find(defenderType) != defenderMap.end()) {
            return defenderMap.at(defenderType);
        }
    }
    
    // 默认效果倍率为1.0
    return 1.0f;
}

void Meowmon::addSkill(const Skill& skill) {
    if (skills.size() < 4) {
        skills.push_back(skill);
    }
}

bool Meowmon::isAlive() const {
    return currentHealth > 0;
}

void Meowmon::initStats() {
    // 根据等级初始化属性
    maxHealth = 50 + (level * 10) + (rand() % (level * 2));
    currentHealth = maxHealth;
    attackValue = 30 + (level * 2) + (rand() % (level + 1));
    defense = 30 + (level * 2) + (rand() % (level + 1));
    speed = 30 + (level * 2) + (rand() % (level + 1));
    
    // 计算下一级所需经验
    nextLevelExp = level * 100;
}
