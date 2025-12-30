#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include "player.h"
#include "enemy.h"
#include "common.h"

// 初始化子弹数组
void initBullets(Bullet bullets[]);

// 玩家发射子弹
void firePlayerBullet(Player* player, Bullet bullets[],int max_bullets);

// 敌人发射子弹
void fireEnemyBullet(Player* player, Bullet bullets[],Enemy enemies[],int i,int max_bullets);

// 更新子弹位置
void updateBullets(Bullet bullets[], int max_bullets, Player* player, Enemy enemies[], int max_enemies);

// 检测子弹是否碰撞目标
bool checkBulletCollision(SDL_Rect* bulletRect, SDL_Rect* targetRect);

#endif
