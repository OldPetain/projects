#include "../include/player.h"
#include "../include/bullet.h"
#include <SDL2/SDL_image.h>

void initPlayer(SDL_Renderer *renderer, Player *player)
{
    // 初始化玩家矩形和生命值
    player->rect.x = SCREEN_WIDTH / 2 - 125;
    player->rect.y = SCREEN_HEIGHT / 2 - 125;
    player->rect.w = 125;
    player->rect.h = 125;
    player->lives = 30;

    // 初始化玩家移动方向
    player->dx = 0;
    player->dy = 0;
    player->speed = 5;
    player->direction = 2; // 0:向上，1：向右上 2:向右 3：向右下 4：向下 5：向左下 6：向左 7：向左上

    // 初始化玩家状态
    player->shooting = false;
    player->jumping = false;
    player->is_jumping = false;

    // 初始化玩家动画
    player->texture = IMG_LoadTexture(renderer, "../assets/player.png"); // 加载精灵图
    player->frame = 0;                                                   // 初始帧索引
    player->frameCount = 1;                                              // 当前动画帧数 (例如站立动画)
    player->frameWidth = 150;                                            // 单帧宽度
    player->frameHeight = 150;                                           // 单帧高度
    player->animationSpeed = 3;                                          // 每3帧切换动画
    player->frameTimer = 0;                                              // 初始化帧计时器
    player->state = STAND_RIGHT;                                         // 初始状态为站立
}

void updatePlayer(Player *player, Bullet bullets[], int max_bullets)
{
    player->frameTimer++;
    if (player->frameTimer >= player->animationSpeed)
    {
        // 切换下一帧
        player->frame = (player->frame + 1) % player->frameCount; //!?
        player->frameTimer = 0;                                   // 重置帧计时器
    }

    // 跳跃逻辑
    if (player->jumping)
    { // 起跳
        player->is_jumping = true;
        player->dy = -PLAYER_JUMP_SPEED;
        player->jumping = false;
    }
    // 播放跳跃动画
    if (player->is_jumping)
    { // 正在腾空
        player->dy += PLAYER_GRAVITY;
        if (player->rect.y + player->rect.h > SCREEN_HEIGHT / 2)// !？？？
        {        
            // TODO:根据SourceRect.x修改地面                                                
            player->rect.y = SCREEN_HEIGHT / 2 - player->rect.h; // 落地
            player->dy = 0;
            player->is_jumping = false;
        }
    }
    // 更新玩家位置
    // 播放人物移动动画：dx+ -->; dx- <--
    player->rect.x += player->dx;
    player->rect.y += player->dy;

    // 边界检测
    if (player->rect.x < 0)
        player->rect.x = 0;
    if (player->rect.x > SCREEN_WIDTH - player->rect.w)
        player->rect.x = SCREEN_WIDTH - player->rect.w;
    if (player->rect.y < 0)
        player->rect.y = 0;
    if (player->rect.y > SCREEN_HEIGHT - player->rect.h)
        player->rect.y = SCREEN_HEIGHT - player->rect.h;

    // 发射子弹
    if (player->shooting)
    {
        firePlayerBullet(player, bullets, max_bullets);
    }
}