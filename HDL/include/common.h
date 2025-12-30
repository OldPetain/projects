// common.h
#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>

#define MAX_ENEMIES 5      // 最大敌人数
#define SCREEN_WIDTH 1200  // 屏幕宽度
#define SCREEN_HEIGHT 765  // 屏幕高度
#define MAP_WIDTH 11374    // 地图宽度
#define MAP_EDGE_WIDTH 200 // 地图边缘宽度

#define MAX_BULLETS 50       // 最大子弹数
#define BULLET_SPEED 10      // 子弹速度
#define PLAYER_JUMP_SPEED 60 // 玩家跳跃初始速度
#define PLAYER_GRAVITY 10    // 玩家重力加速度

typedef enum
{
    STAND_LEFT,
    STAND_RIGHT,
    WALK_LEFT,
    WALK_RIGHT,
    JUMP_LEFT,
    JUMP_RIGHT,
    STAND_UP_RIGHT,
    STAND_UP_LEFT,
    LIE_DOWN_LEFT,
    LIE_DOWN_RIGHT,
    SHOOTING_RUN_RIGHT,
    SHOOTING_RUN_LEFT,
    DIE
} AnimationState;

typedef struct
{
    SDL_Rect rect;   // 玩家矩形
    int dx, dy;      // 玩家移动方向
    int speed;       // 移动速度
    bool shooting;   // 是否射击
    int lives;       // 玩家生命数
    bool jumping;    // 是否跳跃
    bool is_jumping; // 是否处于跳跃状态
    int direction;   // 玩家方向

    // 以下为动画相关
    int frame;            // 当前动画帧索引
    int frameCount;       // 当前动作的总帧数
    int frameWidth;       // 单个动画帧的宽度
    int frameHeight;      // 单个动画帧的高度
    int animationSpeed;   // 动画切换速度（帧计数）
    int frameTimer;       // 帧计时器
    SDL_Texture *texture; // 精灵图纹理
    AnimationState state; // 当前动画状态（站立、跑动、跳跃等）
} Player;

enum enemy_state
{
    RUN_LEFT,
    RUN_RIGHT,
    ENEMY_DIE
};
typedef struct
{

    SDL_Rect rect;          // 敌人矩形
    int dx, dy;             // 敌人速度
    bool active;            // 敌人是否活动
    int direction;          // 敌人方向
    bool dying;             // 敌人是否正在消失
    int deathTimer;         // 死亡动画计时器
    SDL_Texture *texture;   // 精灵图纹理
    enum enemy_state state; // 敌人状态
    int current_frame;      // 当前动画帧索引
    int frame_count;        // 当前动作的总帧数
    int animation_speed;    // 动画切换速度（帧计数）
    int frame_timer;        // 帧计时器
    int frame_width;
    int frame_height;
} Enemy;

typedef struct
{
    SDL_Rect rect;        // 子弹矩形
    int dx, dy;           // 子弹速度
    bool active;          // 子弹是否激活
    int source;           // 子弹来源（玩家或敌人）
    int frame;            // 当前动画帧索引
    int frameCount;       // 当前动作的总帧数
    int animationSpeed;   // 动画切换速度（帧计数）
    int frameTimer;       // 帧计时器
    SDL_Texture *texture; // 精灵图纹理
} Bullet;

// 声明全局变量
extern SDL_Rect SourceRect;
extern SDL_Renderer *renderer;
extern Bullet playerBullets[]; // 玩家子弹
extern Bullet enemyBullets[];  // 敌人子弹

#endif // COMMON_H
