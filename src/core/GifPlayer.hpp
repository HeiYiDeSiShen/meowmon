#ifndef GIFPLAYER_HPP
#define GIFPLAYER_HPP

#include <raylib.h>
#include <string>
#include <vector>

class GifPlayer {
private:
    std::string filePath;
    Image gifImage;
    Texture2D texture;
    int totalFrames;
    int currentFrame;
    float frameTime;
    float frameDuration;
    bool isLoaded;
    bool isPlaying;
    
public:
    GifPlayer();
    ~GifPlayer();
    
    // 加载GIF文件
    bool load(const std::string& path, float fps = 10.0f);
    
    // 更新动画
    void update(float deltaTime);
    
    // 绘制
    void draw(int x, int y, Color tint = WHITE);
    void drawEx(int x, int y, float scale, Color tint = WHITE);
    
    // 控制
    void play();
    void pause();
    void stop();
    void reset();
    
    // 设置
    void setSpeed(float fps);
    void setFrame(int frame);
    
    // 获取信息
    int getCurrentFrame() const { return currentFrame; }
    int getTotalFrames() const { return totalFrames; }
    bool isPlayingState() const { return isPlaying; }
    bool isLoadedState() const { return isLoaded; }
    int getWidth() const { return texture.width; }
    int getHeight() const { return texture.height; }
    
    // 检查是否有效
    bool isValid() const { return isLoaded && totalFrames > 0; }
};

#endif // GIFPLAYER_HPP
