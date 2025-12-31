#ifndef CATCOLLECTION_HPP
#define CATCOLLECTION_HPP

#include <raylib.h>
#include <vector>
#include <string>
#include "../entities/Cat.hpp"

struct CollectionItem {
    CatType type;
    std::string name;
    bool discovered;
    int count;
};

class CatCollection {
public:
    CatCollection();
    ~CatCollection();

    void update(float deltaTime);
    void draw(bool useChinese);
    
    void addCat(CatType type, const std::string& name);
    bool isVisible() const { return visible; }
    void setVisible(bool v) { visible = v; }
    void show() { visible = true; isObserving = false; }
    void hide() { visible = false; }
    void toggle() { if (visible) hide(); else show(); }

private:
    std::vector<CollectionItem> items;
    bool visible;
    bool isObserving; // 是否处于观察详情模式
    int selectedIndex;
    float scrollOffset;
    float observationTimer; // 用于观察模式下的动画
    
    void initCollection();
    void drawCard(const CollectionItem& item, float x, float y, bool selected, bool useChinese);
    void drawObservationView(const CollectionItem& item, bool useChinese);
};

#endif // CATCOLLECTION_HPP
