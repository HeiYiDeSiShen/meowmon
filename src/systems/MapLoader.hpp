#ifndef MAPLOADER_HPP
#define MAPLOADER_HPP

#include <string>
#include <vector>
#include <raylib.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <sstream>
#include <regex>

// 图块结构
struct Tile {
    int id;
    int gid;
    bool isCollidable;
    Vector2 position;
    Rectangle rect;
};

// 图层结构
struct Layer {
    std::string name;
    int width;
    int height;
    std::vector<Tile> tiles;
    bool visible;
    float opacity;
};

// 地图解析器类
class MapLoader {
public:
    // 构造函数
    MapLoader();
    
    // 加载地图
    bool loadMap(const std::string& filePath);
    
    // 绘制地图
    void draw();
    
    // 检查碰撞
    bool checkCollision(const Rectangle& rect) const;
    
    // 获取地图宽度（像素）
    int getMapWidth() const;
    
    // 获取地图高度（像素）
    int getMapHeight() const;
    
    // 获取图块大小
    int getTileSize() const;
    
    // 获取所有图层
    const std::vector<Layer>& getLayers() const;
    
    // 获取指定名称的图层
    const Layer* getLayer(const std::string& name) const;
    
private:
    // 解析地图数据
    bool parseMapData(const std::string& jsonData);
    
    // 解析图层数据
    bool parseLayers(const rapidjson::Value& layersArray);
    
    // 解析图块集数据
    bool parseTilesets(const rapidjson::Value& tilesetsArray, const std::string& mapFilePath);
    
    // TMX格式支持
    bool loadTMX(const std::string& filePath);
    bool loadTSX(const std::string& filePath, int firstGid);
    
    // 辅助函数：解析TMX文件内容
    bool parseTMXContent(const std::string& content, const std::string& filePath);
    bool parseTSXContent(const std::string& content, int firstGid, const std::string& filePath);
    
    // 地图属性
    int width;          // 地图宽度（图块数）
    int height;         // 地图高度（图块数）
    int tileWidth;      // 图块宽度（像素）
    int tileHeight;     // 图块高度（像素）
    std::string orientation;  // 地图方向
    std::string renderOrder;  // 渲染顺序
    
    // 图层
    std::vector<Layer> layers;
    
    // 图块集信息
    std::vector<Texture2D> tilesetTextures;
    std::vector<int> tilesetFirstGids;
    std::vector<std::string> tilesetNames;
};

#endif // MAPLOADER_HPP
