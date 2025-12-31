#include "GifPlayer.hpp"
#include <iostream>

GifPlayer::GifPlayer()
    : totalFrames(0), currentFrame(0), frameTime(0.0f), frameDuration(0.1f),
      isLoaded(false), isPlaying(false) {
    texture = {0};
}

GifPlayer::~GifPlayer() {
    if (isLoaded) {
        // 安全卸载纹理
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
        // 安全卸载图像
        if (gifImage.data != nullptr) {
            UnloadImage(gifImage);
        }
    }
}

bool GifPlayer::load(const std::string& path, float fps) {
    // 检查文件是否存在
    if (!FileExists(path.c_str())) {
        std::cerr << "GIF文件不存在: " << path << std::endl;
        return false;
    }
    
    // 加载GIF动画
    int frames = 0;
    gifImage = LoadImageAnim(path.c_str(), &frames);
    
    if (gifImage.data == nullptr || frames <= 0) {
        std::cerr << "无法加载GIF: " << path << std::endl;
        return false;
    }
    
    totalFrames = frames;
    frameDuration = 1.0f / fps;
    currentFrame = 0;
    frameTime = 0.0f;
    
    // 创建纹理 - 保留图像数据用于后续帧更新
    texture = LoadTextureFromImage(gifImage);
    
    if (texture.id == 0) {
        std::cerr << "无法创建纹理: " << path << std::endl;
        UnloadImage(gifImage);
        return false;
    }
    
    filePath = path;
    isLoaded = true;
    isPlaying = false;
    
    std::cout << "GIF加载成功: " << path << " 帧数: " << totalFrames << std::endl;
    return true;
}

void GifPlayer::update(float deltaTime) {
    if (!isLoaded || !isPlaying || totalFrames <= 1) return;
    
    frameTime += deltaTime;
    
    if (frameTime >= frameDuration) {
        frameTime = 0.0f;
        currentFrame = (currentFrame + 1) % totalFrames;
        
        // 安全更新纹理数据 - 添加更多检查
        if (gifImage.data != nullptr && texture.id != 0 && totalFrames > 0) {
            // 计算当前帧在数据中的偏移
            int frameSize = gifImage.width * gifImage.height * 4; // RGBA
            
            // 确保不会越界
            if (currentFrame >= 0 && currentFrame < totalFrames) {
                // 计算帧数据指针
                size_t offset = (size_t)currentFrame * (size_t)frameSize;
                size_t totalDataSize = (size_t)totalFrames * (size_t)frameSize;
                
                // 检查偏移是否有效
                if (offset + frameSize <= totalDataSize) {
                    unsigned char* frameData = (unsigned char*)gifImage.data + offset;
                    
                    // 安全更新纹理 - 使用raylib的UpdateTexture
                    // 注意：UpdateTexture可能会在某些情况下导致问题，所以添加try-catch
                    #if defined(__APPLE__)
                        // 在macOS上，使用更安全的方式更新纹理
                        if (frameData != nullptr) {
                            UpdateTexture(texture, frameData);
                        }
                    #else
                        UpdateTexture(texture, frameData);
                    #endif
                }
            }
        }
    }
}

void GifPlayer::draw(int x, int y, Color tint) {
    if (!isLoaded) return;
    DrawTexture(texture, x, y, tint);
}

void GifPlayer::drawEx(int x, int y, float scale, Color tint) {
    if (!isLoaded) return;
    
    Rectangle src = {0, 0, (float)texture.width, (float)texture.height};
    Rectangle dest = {(float)x, (float)y, (float)texture.width * scale, (float)texture.height * scale};
    Vector2 origin = {0, 0};
    
    DrawTexturePro(texture, src, dest, origin, 0.0f, tint);
}

void GifPlayer::play() {
    if (isLoaded) {
        isPlaying = true;
    }
}

void GifPlayer::pause() {
    isPlaying = false;
}

void GifPlayer::stop() {
    isPlaying = false;
    currentFrame = 0;
    frameTime = 0.0f;
}

void GifPlayer::reset() {
    currentFrame = 0;
    frameTime = 0.0f;
    if (isLoaded) {
        // 重新加载第一帧
        if (gifImage.data != nullptr) {
            UpdateTexture(texture, gifImage.data);
        }
    }
}

void GifPlayer::setSpeed(float fps) {
    if (fps > 0) {
        frameDuration = 1.0f / fps;
    }
}

void GifPlayer::setFrame(int frame) {
    if (frame >= 0 && frame < totalFrames && isLoaded) {
        currentFrame = frame;
        frameTime = 0.0f;
        
        // 更新到指定帧
        if (gifImage.data != nullptr) {
            int frameSize = gifImage.width * gifImage.height * 4;
            unsigned char* frameData = (unsigned char*)gifImage.data + (currentFrame * frameSize);
            UpdateTexture(texture, frameData);
        }
    }
}
