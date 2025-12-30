#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include "player.h"
#include "enemy.h"
#include "common.h"

// 初始化 SDL
bool initSDL();

// 渲染内容
void render(Player *player, Enemy enemies[], Bullet playerBullets[], Bullet enemyBullets[]);

// 加载背景图片
bool loadBackground(const char* imagePath);

// 清理 SDL
void closeSDL(Player *player);

#endif