#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

enum class GameState {
    START_SCREEN,    // 开始界面
    PLAYING,         // 游戏进行中
    PAUSED,          // 暂停
    GAME_OVER,       // 游戏结束
    SETTINGS,        // 设置菜单
    COLLECTION       // 猫咪图鉴
};

#endif // GAMESTATE_HPP