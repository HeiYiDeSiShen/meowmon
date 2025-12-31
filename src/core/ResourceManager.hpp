#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <string>
#include <unordered_map>
#include <raylib.h>

class ResourceManager {
public:
    // 获取单例实例
    static ResourceManager& getInstance();
    
    // 加载并缓存纹理
    Texture2D loadTexture(const std::string& path);
    
    // 获取已加载的纹理
    Texture2D getTexture(const std::string& path);
    
    // 加载并缓存音效
    Sound loadSound(const std::string& path);
    
    // 获取已加载的音效
    Sound getSound(const std::string& path);
    
    // 加载并缓存音乐
    Music loadMusic(const std::string& path);
    
    // 获取已加载的音乐
    Music getMusic(const std::string& path);
    
    // 加载并缓存字体
    Font loadFont(const std::string& path, int fontSize = 32);
    
    // 获取已加载的字体
    Font getFont(const std::string& path, int fontSize = 32);
    
    // 释放所有资源
    void unloadAll();
    
private:
    // 单例模式：私有构造函数和析构函数
    ResourceManager();
    ~ResourceManager();
    
    // 禁止拷贝和赋值
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // 辅助方法：寻找有效的资源路径
    std::string findValidPath(const std::string& path);
    
    // 资源缓存
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Music> music;
    std::unordered_map<std::string, Font> fonts;
};

#endif // RESOURCEMANAGER_HPP
