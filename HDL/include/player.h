#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include "common.h"

// 初始化玩家
void initPlayer(SDL_Renderer* renderer,Player* player);

// 更新玩家位置
void updatePlayer(Player* player, Bullet bullets[], int max_bullets);

#endif