#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "player.h"
#include "enemy.h"
#include "common.h"

// 辅助函数：更新地图和敌人位置
void updateMapAndEnemies(Player *player, Enemy enemies[], int speed, bool moveLeft);

// 辅助函数：设置玩家状态
void setPlayerState(Player *player, int direction, AnimationState state, int frameCount, int dx, int dy);

// 处理输入事件
bool handleInput(Player* player,Enemy enemies[]);

#endif