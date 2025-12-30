#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "../include/graphics.h"
#include "../include/input.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/bullet.h"
#include "../include/main.h"

#define FR 30        // 帧率
#define FT 1000 / FR // 帧间隔

int main(int argc, char *argv[])
{
    // 初始化SDL
    if (!initSDL())
    {
        IMG_Quit(); // 释放图形模块
        SDL_Quit(); // 释放SDL
        return -1;
    }

    // 初始化玩家
    Player player;
    initPlayer(renderer, &player);

    // 初始化敌人
    Enemy enemies[MAX_ENEMIES];
    initEnemies(enemies);

    // 游戏主循环
    bool running = true;
    Uint64 start, end;
    int delay;
    int flag = 0;
    while (running)
    {
        // 获取当前时间
        start = SDL_GetTicks64();

        // 处理输入
        running = handleInput(&player, enemies);

        // 更新玩家和敌人
        updatePlayer(&player, playerBullets, MAX_BULLETS);
        updateEnemies(enemies, &player, enemyBullets, MAX_BULLETS);

        // 更新子弹
        // Bullet bullets[], int max_bullets, Player* player, Enemy enemies[], int max_enemies
        updateBullets(playerBullets, MAX_BULLETS, &player, enemies, MAX_ENEMIES);
        updateBullets(enemyBullets, MAX_BULLETS, &player, enemies, MAX_ENEMIES);

        // 检测人物碰撞
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (checkCollision(&player.rect, &enemies[i].rect))
            {
                player.lives--;
                if (player.lives <= 0)
                {
                    running = false;
                }
            }
        }

        // 胜利条件
        if (SourceRect.x + player.rect.x + player.rect.w >= 11000)
        {
            printf("You Win!\n");
            flag = 1;
            running = false;
        }

        // 渲染内容
        render(&player, enemies, playerBullets, enemyBullets);

        // 延迟
        end = SDL_GetTicks64();
        delay = FT - (end - start);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }
    /*
    TTF_Font *font = NULL;

    int texW = 0;
    int texH = 0;
    SDL_Texture* texture_1 = IMG_LoadTexture("win.png"); // load winning text
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    if(flag == 0){
    }
    */
    // if (flag == 1)
    // {
    //     if (TTF_Init() == -1)
    //     {
    //         printf("TTF_Init: %s\n", TTF_GetError());
    //         return -1;
    //     }
    //     TTF_Font *font = TTF_OpenFont("font.ttf", 32);
    //     if (font == NULL)
    //     {
    //         printf("TTF_OpenFont: %s\n", TTF_GetError());
    //         return -1;
    //     }
    //     SDL_Color color = {255, 255, 255, 255}; // white color
    //     SDL_Surface *txt_surf = TTF_RenderText_Blended(font, "You Win!", color);
    //     SDL_Surface *surf = SDL_GetWindowSurface(win);
    //     SDL_Rect rect = {300, 200, txt_surf->w, txt_surf->h};
    //     SDL_BlitSurface(txt_surf, NULL, surf, &rect);
    //     SDL_UpdateWindowSurface(win);
    //     SDL_FreeSurface(txt_surf);
    //     TTF_CloseFont(font);
    //     SDL_Delay(3000);
    // }

    // 清理资源
    closeSDL(&player);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
