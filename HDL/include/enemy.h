#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "player.h"
#include "common.h"

// 初始化敌人
void initEnemies(Enemy enemies[]);

// 更新敌人状态（位置等）
void updateEnemies(Enemy enemies[],Player* player, Bullet bullets[], int max_bullets);

// 碰撞检测
bool checkCollision(SDL_Rect* a, SDL_Rect* b);

#endif