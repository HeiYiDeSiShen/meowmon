#include "MapLoader.hpp"
#include "core/ResourceManager.hpp"
#include <iostream>
#include <sstream>

MapLoader::MapLoader() 
    : width(0), height(0), tileWidth(0), tileHeight(0),
      orientation("orthogonal"), renderOrder("right-down") {
}

bool MapLoader::loadMap(const std::string& filePath) {
    // 检查文件扩展名来决定使用哪种格式
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    
    if (extension == "tmx" || extension == "TMX") {
        return loadTMX(filePath);
    } else if (extension == "json" || extension == "JSON") {
        // 使用现有的JSON解析
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法打开地图文件: " << filePath << std::endl;
            return false;
        }
        
        rapidjson::IStreamWrapper isw(file);
        rapidjson::Document doc;
        doc.ParseStream(isw);
        
        if (doc.HasParseError()) {
            std::cerr << "地图文件解析错误: " << doc.GetParseError() << std::endl;
            return false;
        }
        
        // 解析地图基本属性
        if (doc.HasMember("width")) width = doc["width"].GetInt();
        if (doc.HasMember("height")) height = doc["height"].GetInt();
        if (doc.HasMember("tilewidth")) tileWidth = doc["tilewidth"].GetInt();
        if (doc.HasMember("tileheight")) tileHeight = doc["tileheight"].GetInt();
        if (doc.HasMember("orientation")) orientation = doc["orientation"].GetString();
        if (doc.HasMember("renderorder")) renderOrder = doc["renderorder"].GetString();
        
        std::cout << "地图尺寸: " << width << "x" << height << " 图块大小: " << tileWidth << "x" << tileHeight << std::endl;
        
        // 解析图块集
        if (doc.HasMember("tilesets")) {
            if (!parseTilesets(doc["tilesets"], filePath)) {
                std::cerr << "解析图块集失败!" << std::endl;
                return false;
            }
        }
        
        // 解析图层
        if (doc.HasMember("layers")) {
            if (!parseLayers(doc["layers"])) {
                std::cerr << "解析图层失败!" << std::endl;
                return false;
            }
        }
        
        std::cout << "成功加载地图: " << filePath << std::endl;
        return true;
    } else {
        std::cerr << "不支持的地图格式: " << extension << std::endl;
        return false;
    }
}

bool MapLoader::parseTilesets(const rapidjson::Value& tilesetsArray, const std::string& filePath) {
    if (!tilesetsArray.IsArray()) return false;
    
    for (rapidjson::SizeType i = 0; i < tilesetsArray.Size(); ++i) {
        const rapidjson::Value& tileset = tilesetsArray[i];
        
        // 获取图块集基本信息
        int firstGid = tileset["firstgid"].GetInt();
        
        // 检查是否有name字段，如果没有使用默认名称
        std::string name = "tileset";
        if (tileset.HasMember("name")) {
            name = tileset["name"].GetString();
        }
        
        // 解析图块集图片
        if (tileset.HasMember("image")) {
            std::string imagePath = tileset["image"].GetString();
            
            // 尝试加载图块集纹理
            try {
                Texture2D texture = ResourceManager::getInstance().loadTexture(imagePath);
                tilesetTextures.push_back(texture);
                tilesetFirstGids.push_back(firstGid);
                tilesetNames.push_back(name);
            } catch (const std::exception& e) {
                std::cerr << "无法加载图块集图片: " << imagePath << " 错误: " << e.what() << std::endl;
                return false;
            }
        }
        // 如果是引用外部tileset文件（Tiled格式）
        else if (tileset.HasMember("source")) {
            std::string sourcePath = tileset["source"].GetString();
            
            // 构建完整的tileset图片路径
            std::string directoryPath = filePath.substr(0, filePath.find_last_of("/\\"));
            std::string imagePath;
            
            // 处理source路径中的../引用
            if (sourcePath.find("../") == 0) {
                std::string parentDir = directoryPath.substr(0, directoryPath.find_last_of("/\\"));
                imagePath = parentDir + "/" + sourcePath.substr(3);
            } else {
                imagePath = directoryPath + "/" + sourcePath;
            }
            
            // 尝试加载tileset图片
            try {
                Texture2D texture = ResourceManager::getInstance().loadTexture(imagePath);
                tilesetTextures.push_back(texture);
                tilesetFirstGids.push_back(firstGid);
                tilesetNames.push_back(name);
            } catch (const std::exception& e) {
                std::cerr << "无法加载tileset图片: " << imagePath << " 错误: " << e.what() << std::endl;
                // 创建占位纹理
                Image image = GenImageColor(32, 32, GRAY);
                Texture2D texture = LoadTextureFromImage(image);
                UnloadImage(image);
                
                tilesetTextures.push_back(texture);
                tilesetFirstGids.push_back(firstGid);
                tilesetNames.push_back(name);
            }
        }
    }
    
    return true;
}

bool MapLoader::parseLayers(const rapidjson::Value& layersArray) {
    if (!layersArray.IsArray()) return false;
    
    for (rapidjson::SizeType i = 0; i < layersArray.Size(); ++i) {
        const rapidjson::Value& layerValue = layersArray[i];
        
        Layer layer;
        
        // 解析图层基本信息
        if (layerValue.HasMember("name")) layer.name = layerValue["name"].GetString();
        if (layerValue.HasMember("width")) layer.width = layerValue["width"].GetInt();
        if (layerValue.HasMember("height")) layer.height = layerValue["height"].GetInt();
        if (layerValue.HasMember("visible")) layer.visible = layerValue["visible"].GetBool();
        else layer.visible = true;
        if (layerValue.HasMember("opacity")) layer.opacity = layerValue["opacity"].GetFloat();
        else layer.opacity = 1.0f;
        
        // 解析图块数据
        if (layerValue.HasMember("data") && layerValue["data"].IsArray()) {
            const rapidjson::Value& dataArray = layerValue["data"];
            
            for (rapidjson::SizeType j = 0; j < dataArray.Size(); ++j) {
                const rapidjson::Value& dataValue = dataArray[j];
                int gid = 0;
                
                if (dataValue.IsInt()) {
                    gid = dataValue.GetInt();
                } else if (dataValue.IsInt64()) {
                    gid = static_cast<int>(dataValue.GetInt64());
                } else if (dataValue.IsString()) {
                    std::string str = dataValue.GetString();
                    gid = std::stoi(str);
                } else {
                    std::cerr << "警告: 无法解析图块数据类型" << std::endl;
                    continue;
                }
                
                if (gid == 0) continue;
                
                // 处理 Tiled 的翻转标志位
                const int FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                const int FLIPPED_VERTICALLY_FLAG = 0x40000000;
                const int FLIPPED_DIAGONALLY_FLAG = 0x20000000;
                const int ROTATED_HEXAGONAL_120_FLAG = 0x10000000;
                
                int realGid = gid & ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG | ROTATED_HEXAGONAL_120_FLAG);
                
                if (realGid == 0) continue;
                
                int x = j % layer.width;
                int y = j / layer.width;
                
                Tile tile;
                tile.id = static_cast<int>(j);
                tile.gid = realGid;
                tile.position = {static_cast<float>(x * tileWidth), static_cast<float>(y * tileHeight)};
                tile.rect = {tile.position.x, tile.position.y, static_cast<float>(tileWidth), static_cast<float>(tileHeight)};
                tile.isCollidable = (realGid > 100);
                
                layer.tiles.push_back(tile);
            }
        }
        
        layers.push_back(layer);
    }
    
    return true;
}

// TMX格式解析器 - 使用字符串解析
bool MapLoader::loadTMX(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "无法打开TMX文件: " << filePath << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return parseTMXContent(content, filePath);
}

bool MapLoader::loadTSX(const std::string& filePath, int firstGid) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "无法打开TSX文件: " << filePath << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return parseTSXContent(content, firstGid, filePath);
}

bool MapLoader::parseTMXContent(const std::string& content, const std::string& filePath) {
    // 简单的字符串解析来提取地图属性
    size_t mapPos = content.find("<map");
    if (mapPos == std::string::npos) return false;
    
    // 提取宽度、高度、图块大小
    size_t widthPos = content.find("width=\"", mapPos);
    size_t heightPos = content.find("height=\"", mapPos);
    size_t tilewidthPos = content.find("tilewidth=\"", mapPos);
    size_t tileheightPos = content.find("tileheight=\"", mapPos);
    
    if (widthPos != std::string::npos && heightPos != std::string::npos && 
        tilewidthPos != std::string::npos && tileheightPos != std::string::npos) {
        
        size_t widthEnd = content.find("\"", widthPos + 7);
        size_t heightEnd = content.find("\"", heightPos + 8);
        size_t tilewidthEnd = content.find("\"", tilewidthPos + 11);
        size_t tileheightEnd = content.find("\"", tileheightPos + 12);
        
        width = std::stoi(content.substr(widthPos + 7, widthEnd - widthPos - 7));
        height = std::stoi(content.substr(heightPos + 8, heightEnd - heightPos - 8));
        tileWidth = std::stoi(content.substr(tilewidthPos + 11, tilewidthEnd - tilewidthPos - 11));
        tileHeight = std::stoi(content.substr(tileheightPos + 12, tileheightEnd - tileheightPos - 12));
        
        std::cout << "TMX地图尺寸: " << width << "x" << height << " 图块大小: " << tileWidth << "x" << tileHeight << std::endl;
    } else {
        return false;
    }
    
    // 解析tileset
    size_t tilesetPos = content.find("<tileset");
    while (tilesetPos != std::string::npos) {
        size_t nextTagPos = content.find(">", tilesetPos);
        size_t sourcePos = content.find("source=\"", tilesetPos);
        size_t firstgidPos = content.find("firstgid=\"", tilesetPos);
        
        int firstGid = 1;
        if (firstgidPos != std::string::npos && (nextTagPos == std::string::npos || firstgidPos < nextTagPos)) {
            size_t firstgidEnd = content.find("\"", firstgidPos + 10);
            firstGid = std::stoi(content.substr(firstgidPos + 10, firstgidEnd - firstgidPos - 10));
        }

        // 检查是否是外部 TSX 文件
        if (sourcePos != std::string::npos && (nextTagPos == std::string::npos || sourcePos < nextTagPos)) {
            size_t sourceEnd = content.find("\"", sourcePos + 8);
            std::string source = content.substr(sourcePos + 8, sourceEnd - sourcePos - 8);
            
            std::string directoryPath = filePath.substr(0, filePath.find_last_of("/\\"));
            std::string tsxPath = directoryPath + "/" + source;
            
            if (!loadTSX(tsxPath, firstGid)) {
                std::cerr << "无法加载外部 TSX 文件: " << tsxPath << std::endl;
            }
        } else {
            // 嵌入式 tileset，寻找 <image> 标签
            size_t imageTagPos = content.find("<image", tilesetPos);
            size_t tilesetEndPos = content.find("</tileset>", tilesetPos);
            
            if (imageTagPos != std::string::npos && (tilesetEndPos == std::string::npos || imageTagPos < tilesetEndPos)) {
                size_t imageSourcePos = content.find("source=\"", imageTagPos);
                if (imageSourcePos != std::string::npos) {
                    size_t imageSourceEnd = content.find("\"", imageSourcePos + 8);
                    std::string imageSource = content.substr(imageSourcePos + 8, imageSourceEnd - imageSourcePos - 8);
                    
                    std::string directoryPath = filePath.substr(0, filePath.find_last_of("/\\"));
                    std::string imagePath = directoryPath + "/" + imageSource;
                    
                    try {
                        Texture2D texture = ResourceManager::getInstance().loadTexture(imagePath);
                        tilesetTextures.push_back(texture);
                        tilesetFirstGids.push_back(firstGid);
                        
                        // 提取名称
                        std::string name = "embedded_tileset";
                        size_t namePos = content.find("name=\"", tilesetPos);
                        if (namePos != std::string::npos && namePos < nextTagPos) {
                            size_t nameEnd = content.find("\"", namePos + 6);
                            name = content.substr(namePos + 6, nameEnd - namePos - 6);
                        }
                        tilesetNames.push_back(name);
                        std::cout << "加载嵌入式图块集: " << name << " 图片: " << imagePath << std::endl;
                    } catch (...) {
                        std::cerr << "无法加载嵌入式图块集图片: " << imagePath << std::endl;
                    }
                }
            }
        }
        
        tilesetPos = content.find("<tileset", tilesetPos + 1);
    }
    
    // 解析图层
    size_t layerPos = content.find("<layer");
    while (layerPos != std::string::npos) {
        Layer layer;
        layer.visible = true;
        layer.opacity = 1.0f;
        
        // 提取图层属性
        size_t namePos = content.find("name=\"", layerPos);
        size_t widthPos = content.find("width=\"", layerPos);
        size_t heightPos = content.find("height=\"", layerPos);
        
        if (namePos != std::string::npos) {
            size_t nameEnd = content.find("\"", namePos + 6);
            layer.name = content.substr(namePos + 6, nameEnd - namePos - 6);
        }
        
        if (widthPos != std::string::npos) {
            size_t widthEnd = content.find("\"", widthPos + 7);
            layer.width = std::stoi(content.substr(widthPos + 7, widthEnd - widthPos - 7));
        }
        
        if (heightPos != std::string::npos) {
            size_t heightEnd = content.find("\"", heightPos + 8);
            layer.height = std::stoi(content.substr(heightPos + 8, heightEnd - heightPos - 8));
        }
        
        // 查找数据部分
        size_t dataPos = content.find("<data", layerPos);
        size_t dataEnd = content.find("</data>", layerPos);
        
        if (dataPos != std::string::npos && dataEnd != std::string::npos) {
            std::string dataContent = content.substr(dataPos, dataEnd - dataPos);
            
            // 提取CSV内容
            size_t csvStart = dataContent.find(">");
            if (csvStart != std::string::npos) {
                std::string csvData = dataContent.substr(csvStart + 1);
                
                // 解析CSV
                std::stringstream ss(csvData);
                std::string token;
                int index = 0;
                
                while (std::getline(ss, token, ',')) {
                    token.erase(0, token.find_first_not_of(" \t\n\r"));
                    token.erase(token.find_last_not_of(" \t\n\r") + 1);
                    
                    if (!token.empty()) {
                        try {
                            // 处理可能的翻转标志位
                            unsigned long long gidValue = std::stoull(token);
                            
                            // 处理 Tiled 的翻转标志位
                            const unsigned long long FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                            const unsigned long long FLIPPED_VERTICALLY_FLAG = 0x40000000;
                            const unsigned long long FLIPPED_DIAGONALLY_FLAG = 0x20000000;
                            const unsigned long long ROTATED_HEXAGONAL_120_FLAG = 0x10000000;
                            
                            unsigned long long realGid = gidValue & ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG | ROTATED_HEXAGONAL_120_FLAG);
                            
                            if (realGid > 0 && realGid <= INT_MAX) {
                                int gid = static_cast<int>(realGid);
                                int x = index % layer.width;
                                int y = index / layer.width;
                                
                                Tile tile;
                                tile.id = index;
                                tile.gid = gid;
                                tile.position = {static_cast<float>(x * tileWidth), static_cast<float>(y * tileHeight)};
                                tile.rect = {tile.position.x, tile.position.y, static_cast<float>(tileWidth), static_cast<float>(tileHeight)};
                                tile.isCollidable = (gid > 100);
                                
                                layer.tiles.push_back(tile);
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "解析图块数据错误: " << token << " - " << e.what() << std::endl;
                        }
                    }
                    
                    index++;
                }
            }
        }
        
        layers.push_back(layer);
        layerPos = content.find("<layer", layerPos + 1);
    }
    
    std::cout << "成功加载TMX地图: " << filePath << std::endl;
    return true;
}

bool MapLoader::parseTSXContent(const std::string& content, int firstGid, const std::string& filePath) {
    // 提取tileset名称
    std::string tilesetName = "tileset";
    size_t namePos = content.find("name=\"");
    if (namePos != std::string::npos) {
        size_t nameEnd = content.find("\"", namePos + 6);
        if (nameEnd != std::string::npos) {
            tilesetName = content.substr(namePos + 6, nameEnd - namePos - 6);
        }
    }
    
    // 提取图片路径
    size_t imagePos = content.find("<image");
    if (imagePos != std::string::npos) {
        size_t sourceStart = content.find("source=\"", imagePos);
        size_t sourceEnd = content.find("\"", sourceStart + 8);
        
        if (sourceStart != std::string::npos && sourceEnd != std::string::npos) {
            std::string source = content.substr(sourceStart + 8, sourceEnd - sourceStart - 8);
            
            // 构建完整图片路径
            std::string directoryPath = filePath.substr(0, filePath.find_last_of("/\\"));
            std::string imagePath;
            
            if (source.find("../") == 0) {
                std::string parentDir = directoryPath.substr(0, directoryPath.find_last_of("/\\"));
                imagePath = parentDir + "/" + source.substr(3);
            } else {
                imagePath = directoryPath + "/" + source;
            }
            
            // 加载纹理
            try {
                Texture2D texture = ResourceManager::getInstance().loadTexture(imagePath);
                tilesetTextures.push_back(texture);
                tilesetFirstGids.push_back(firstGid);
                tilesetNames.push_back(tilesetName);
                
                std::cout << "加载TSX图块集: " << tilesetName << " 图片: " << imagePath << std::endl;
                return true;
            } catch (const std::exception& e) {
                std::cerr << "无法加载TSX图片: " << imagePath << " 错误: " << e.what() << std::endl;
                return false;
            }
        }
    }
    
    std::cerr << "TSX文件格式错误: 缺少image元素" << std::endl;
    return false;
}

void MapLoader::draw() {
    for (const auto& layer : layers) {
        if (!layer.visible) continue;
        
        for (const auto& tile : layer.tiles) {
            int tilesetIndex = -1;
            int tilesetFirstGid = 0;
            
            for (size_t i = 0; i < tilesetFirstGids.size(); ++i) {
                if (tilesetFirstGids[i] <= tile.gid && 
                    (tilesetIndex == -1 || tilesetFirstGids[i] > tilesetFirstGid)) {
                    tilesetIndex = static_cast<int>(i);
                    tilesetFirstGid = tilesetFirstGids[i];
                }
            }
            
            if (tilesetIndex != -1 && tilesetIndex < tilesetTextures.size()) {
                int localGid = tile.gid - tilesetFirstGid;
                Texture2D texture = tilesetTextures[tilesetIndex];
                
                if (tileWidth > 0 && tileHeight > 0 && texture.width > 0 && texture.height > 0) {
                    int tilesPerRow = texture.width / tileWidth;
                    
                    if (tilesPerRow > 0) {
                        int tilesetX = (localGid % tilesPerRow) * tileWidth;
                        int tilesetY = (localGid / tilesPerRow) * tileHeight;
                        
                        if (tilesetX >= 0 && tilesetX + tileWidth <= texture.width &&
                            tilesetY >= 0 && tilesetY + tileHeight <= texture.height) {
                            
                            Rectangle sourceRect = {static_cast<float>(tilesetX), static_cast<float>(tilesetY), 
                                                   static_cast<float>(tileWidth), static_cast<float>(tileHeight)};
                            
                            DrawTextureRec(texture, sourceRect, tile.position, WHITE);
                        }
                    }
                }
            }
        }
    }
}

bool MapLoader::checkCollision(const Rectangle& rect) const {
    for (const auto& layer : layers) {
        if (!layer.visible) continue;
        
        for (const auto& tile : layer.tiles) {
            if (tile.isCollidable) {
                if (CheckCollisionRecs(rect, tile.rect)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

int MapLoader::getMapWidth() const {
    return width * tileWidth;
}

int MapLoader::getMapHeight() const {
    return height * tileHeight;
}

int MapLoader::getTileSize() const {
    return tileWidth;
}

const std::vector<Layer>& MapLoader::getLayers() const {
    return layers;
}

const Layer* MapLoader::getLayer(const std::string& name) const {
    for (const auto& layer : layers) {
        if (layer.name == name) {
            return &layer;
        }
    }
    return nullptr;
}
