#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../include/input.h"
#include "../include/common.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/bullet.h"

// 全局事件
SDL_Event event;

// 辅助函数：更新地图和敌人位置
void updateMapAndEnemies(Player *player, Enemy enemies[], int speed, bool moveLeft)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].rect.x += (moveLeft ? speed : -speed);
    }
}

// 辅助函数：设置玩家状态
void setPlayerState(Player *player, int direction, AnimationState state, int frameCount, int dx, int dy)
{
    player->direction = direction;
    player->state = state;
    player->frameCount = frameCount;
    player->dx = dx;
    player->dy = dy;
}

bool handleInput(Player *player, Enemy enemies[])
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            // 退出事件
            return false;
        }
    }

    const Uint8 *keyState = SDL_GetKeyboardState(NULL);

    // 跳跃逻辑
    if (keyState[SDL_SCANCODE_SPACE] && !player->is_jumping) // 如果按下空格键并且没有在跳跃中
    {
        player->jumping = true;
        setPlayerState(
            player,
            (player->direction == 5 || player->direction == 6 || player->direction == 7) ? 6 : 2,
            (player->direction == 5 || player->direction == 6 || player->direction == 7) ? JUMP_LEFT : JUMP_RIGHT,
            4, 0, -player->speed);
        if (player->state == JUMP_LEFT && keyState[SDL_SCANCODE_A])
        {
            player->dx = -player->speed;
        }
        else if (player->state == JUMP_RIGHT && keyState[SDL_SCANCODE_D])
        {
            player->dx = player->speed;
        }
    }

    // 射击逻辑
    if (keyState[SDL_SCANCODE_J])
    {
        player->shooting = true;
        // firePlayerBullet(player, playerBullets, MAX_BULLETS);
    }
    else
    {
        player->shooting = false;
    }

    // 移动逻辑
    if (!player->jumping && !player->is_jumping)
    {
        if (keyState[SDL_SCANCODE_W] && keyState[SDL_SCANCODE_A])
        {
            setPlayerState(player, 7, WALK_LEFT, 5, -player->speed, 0);
        }
        else if (keyState[SDL_SCANCODE_W] && keyState[SDL_SCANCODE_D])
        {
            setPlayerState(player, 1, WALK_RIGHT, 5, player->speed, 0);
        }
        else if (keyState[SDL_SCANCODE_S] && keyState[SDL_SCANCODE_A])
        {
            setPlayerState(player, 5, LIE_DOWN_LEFT, 1, 0, 0);
        }
        else if (keyState[SDL_SCANCODE_S] && keyState[SDL_SCANCODE_D])
        {
            setPlayerState(player, 3, LIE_DOWN_RIGHT, 1, 0, 0);
        }
        else if (keyState[SDL_SCANCODE_A])
        {
            setPlayerState(player, 6, WALK_LEFT, 5, -player->speed, 0);
        }
        else if (keyState[SDL_SCANCODE_D])
        {
            setPlayerState(player, 2, WALK_RIGHT, 5, player->speed, 0);
        }
        else if (keyState[SDL_SCANCODE_S])
        {
            if (player->direction == 6 || player->state == WALK_LEFT || player->state == STAND_LEFT)
            {
                setPlayerState(player, 4, LIE_DOWN_LEFT, 1, 0, 0); // 趴下左
            }
            else if (player->direction == 2 || player->state == WALK_RIGHT || player->state == STAND_RIGHT)
            {
                setPlayerState(player, 4, LIE_DOWN_RIGHT, 1, 0, 0); // 趴下右
            }
        }
        else if (keyState[SDL_SCANCODE_W])
        {
            if (player->direction == 6 || player->state == WALK_LEFT || player->state == STAND_LEFT)
            {
                setPlayerState(player, 0, STAND_UP_LEFT, 1, 0, 0); // 面向左，向上举枪
            }
            else if (player->direction == 2 || player->state == WALK_RIGHT || player->state == STAND_RIGHT)
            {
                setPlayerState(player, 0, STAND_UP_RIGHT, 1, 0, 0); // 面向右，向上举枪
            }
        }
        else
        {
            // 无按键按下，根据最后方向停止
            if (player->direction == 6 || player->state == WALK_LEFT || player->state == LIE_DOWN_LEFT || player->state == STAND_UP_LEFT)
            {
                setPlayerState(player, 6, STAND_LEFT, 1, 0, 0); // 面向左停止
            }
            else if (player->direction == 2 || player->state == WALK_RIGHT || player->state == LIE_DOWN_RIGHT || player->state == STAND_UP_RIGHT)
            {
                setPlayerState(player, 2, STAND_RIGHT, 1, 0, 0); // 面向右停止
            }
        }
    }
    // TODO:加入我之前判断地图边缘的逻辑
    // 玩家靠近左右地图边缘时，更新地图和敌人位置
    if (player->rect.x < MAP_EDGE_WIDTH) // 靠近左边缘
    {
        if (SourceRect.x < 0)
        {
            SourceRect.x = 0;
        }
        if (player->dx < 0 && SourceRect.x > 0) // 如果玩家在向左移动
        {
            player->dx = 0;                // 玩家停止移动
            SourceRect.x -= player->speed; // 移动地图
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                enemies[i].rect.x += player->speed;
            } // 更新敌人位置
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (playerBullets[i].active)
                {
                    playerBullets[i].rect.x += player->speed;
                }
                if (enemyBullets[i].active)
                {
                    enemyBullets[i].rect.x += player->speed;
                }
            } // 更新子弹位置
        }
    }

    if (player->rect.x + player->rect.w > SCREEN_WIDTH - MAP_EDGE_WIDTH) // 靠近右边缘
    {
        if (SourceRect.x + SCREEN_WIDTH > MAP_WIDTH)
        {
            SourceRect.x = MAP_WIDTH - SCREEN_WIDTH;
        }
        if (player->dx > 0 && SourceRect.x + SCREEN_WIDTH < MAP_WIDTH) // 如果玩家在向右移动
        {
            player->dx = 0;                // 玩家停止移动
            SourceRect.x += player->speed; // 移动地图
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                enemies[i].rect.x -= player->speed;
            } // 更新敌人位置
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (playerBullets[i].active)
                {
                    playerBullets[i].rect.x -= player->speed;
                }
                if (enemyBullets[i].active)
                {
                    enemyBullets[i].rect.x -= player->speed;
                }
            } // 更新子弹位置
        }
    }

    return true;
}