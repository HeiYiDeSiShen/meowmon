#include "ResourceManager.hpp"

ResourceManager::ResourceManager() {
    // 初始化资源管理器
}

ResourceManager::~ResourceManager() {
    // 释放所有资源
    unloadAll();
}

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

std::string ResourceManager::findValidPath(const std::string& path) {
    // 1. 尝试原始路径
    if (FileExists(path.c_str())) return path;
    
    // 2. 尝试加上 ../ (适用于在 build 目录下运行)
    std::string parentPath = "../" + path;
    if (FileExists(parentPath.c_str())) return parentPath;
    
    // 3. 如果是 assets 开头，尝试去掉 assets/ (适用于资源在当前目录)
    if (path.find("assets/") == 0) {
        std::string strippedPath = path.substr(7);
        if (FileExists(strippedPath.c_str())) return strippedPath;
    }
    
    return path; // 如果都找不到，返回原始路径供 raylib 报告错误
}

Texture2D ResourceManager::loadTexture(const std::string& path) {
    // 检查是否已加载
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }
    
    // 加载新纹理
    std::string validPath = findValidPath(path);
    Texture2D texture = LoadTexture(validPath.c_str());
    textures[path] = texture;
    return texture;
}

Texture2D ResourceManager::getTexture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }
    
    // 如果未找到，尝试加载
    return loadTexture(path);
}

Sound ResourceManager::loadSound(const std::string& path) {
    // 检查是否已加载
    auto it = sounds.find(path);
    if (it != sounds.end()) {
        return it->second;
    }
    
    // 加载新音效
    std::string validPath = findValidPath(path);
    Sound sound = LoadSound(validPath.c_str());
    sounds[path] = sound;
    return sound;
}

Sound ResourceManager::getSound(const std::string& path) {
    auto it = sounds.find(path);
    if (it != sounds.end()) {
        return it->second;
    }
    
    // 如果未找到，尝试加载
    return loadSound(path);
}

Music ResourceManager::loadMusic(const std::string& path) {
    // 检查是否已加载
    auto it = music.find(path);
    if (it != music.end()) {
        return it->second;
    }
    
    // 加载新音乐
    std::string validPath = findValidPath(path);
    Music m = LoadMusicStream(validPath.c_str());
    music[path] = m;
    return m;
}

Music ResourceManager::getMusic(const std::string& path) {
    auto it = music.find(path);
    if (it != music.end()) {
        return it->second;
    }
    
    // 如果未找到，尝试加载
    return loadMusic(path);
}

void ResourceManager::unloadAll() {
    // 释放所有纹理
    for (auto& pair : textures) {
        UnloadTexture(pair.second);
    }
    textures.clear();
    
    // 释放所有音效
    for (auto& pair : sounds) {
        UnloadSound(pair.second);
    }
    sounds.clear();
    
    // 释放所有音乐
    for (auto& pair : music) {
        UnloadMusicStream(pair.second);
    }
    music.clear();

    // 释放所有字体
    for (auto& pair : fonts) {
        UnloadFont(pair.second);
    }
    fonts.clear();
}

Font ResourceManager::loadFont(const std::string& path, int fontSize) {
    // 检查是否已加载
    std::string key = path + "_" + std::to_string(fontSize);
    auto it = fonts.find(key);
    if (it != fonts.end()) {
        return it->second;
    }
    
    // 加载新字体
    std::string validPath = findValidPath(path);
    
    // 为了支持中文，我们需要加载特定的 codepoints
    // CJK 统一汉字区间: 0x4E00 - 0x9FFF
    // 加上 ASCII 区间: 0x0020 - 0x007F
    // 加上中文标点符号: 0x3000 - 0x303F, 0xFF00 - 0xFFEF
    
    const int numRanges = 4;
    int ranges[numRanges * 2] = {
        0x0020, 0x007F, // ASCII
        0x4E00, 0x9FFF, // CJK Unified Ideographs
        0x3000, 0x303F, // CJK Symbols and Punctuation
        0xFF00, 0xFFEF  // Halfwidth and Fullwidth Forms
    };
    
    int codepointCount = 0;
    for (int i = 0; i < numRanges; i++) {
        codepointCount += (ranges[i * 2 + 1] - ranges[i * 2] + 1);
    }
    
    int* codepoints = (int*)malloc(codepointCount * sizeof(int));
    int index = 0;
    for (int i = 0; i < numRanges; i++) {
        for (int cp = ranges[i * 2]; cp <= ranges[i * 2 + 1]; cp++) {
            codepoints[index++] = cp;
        }
    }
    
    Font font = LoadFontEx(validPath.c_str(), fontSize, codepoints, codepointCount);
    free(codepoints);
    
    fonts[key] = font;
    return font;
}

Font ResourceManager::getFont(const std::string& path, int fontSize) {
    std::string key = path + "_" + std::to_string(fontSize);
    auto it = fonts.find(key);
    if (it != fonts.end()) {
        return it->second;
    }
    
    // 如果未找到，尝试加载
    return loadFont(path, fontSize);
}
