#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include "../include/graphics.h"
#include "../include/player.h"
#include "../include/enemy.h"
#include "../include/common.h"
#include "../include/bullet.h"

// 全局变量
SDL_Window *win = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *backgroundTexture = NULL;                     // 背景纹理
SDL_Texture *bulletTexture = NULL;                         // 子弹纹理
SDL_Rect SourceRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; // 背景源矩形
SDL_Rect DestRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};   // 背景矩形
Bullet playerBullets[MAX_BULLETS];                         // 玩家子弹
Bullet enemyBullets[MAX_BULLETS];                          // 敌人子弹

bool initSDL()
{
    // 初始化 SDL 视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // 初始化 SDL_image 图像处理库
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    

    // 初始化子弹
    initBullets(playerBullets);
    initBullets(enemyBullets);

    // 创建窗口
    win = SDL_CreateWindow("Contra Game",
                           SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED,
                           SCREEN_WIDTH,
                           SCREEN_HEIGHT,
                           SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!win)
   
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
   
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return false;
    }

    // 加载背景
    if  (!loadBackground("../assets/map.png"))
    
    {
        return false;
    }

    return true;
}

void render(Player *player, Enemy enemies[], Bullet playerBullets[], Bullet enemyBullets[])
{
    // 渲染背景
    SDL_RenderCopy(renderer, backgroundTexture, &SourceRect, &DestRect);

    // 绘制玩家
    SDL_Rect srcRect_player = {
        player->frame * player->frameWidth,  // 源X位置，根据当前帧计算
        player->state * player->frameHeight, // 源Y位置，根据状态计算
        player->frameWidth,                  // 帧宽度
        player->frameHeight                  // 帧高度
    };
    SDL_RenderCopy(renderer, player->texture, &srcRect_player, &player->rect);

    // 绘制敌人（绿色）
    // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 普通敌人颜色：绿色
    // for (int i = 0; i < MAX_ENEMIES; i++)
    // {
    //     if (enemies[i].active)
    //     {
    //         if (enemies[i].dying)
    //         {
    //             SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 敌人被击中变成蓝色
    //             enemies[i].state = ENEMY_DIE;
    //         }
    //         SDL_RenderFillRect(renderer, &(enemies[i].rect));
    //         SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 恢复颜色
    //     }
    // }

    // 绘制敌人
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            std::cout << "enemy" << i << " is died" << std::endl;
        }
        if (enemies[i].active)
        {
            // std::cout << "-----------------enemy" << i << " is active--------------" << std::endl;                     // test
            // std::cout << i << "now || size:" << enemies[i].frame_height << "x" << enemies[i].frame_width << std::endl; // test

            SDL_Rect srcRect2 = {enemies[i].current_frame * enemies[i].frame_width,
                                 enemies[i].state * enemies[i].frame_height,
                                 enemies[i].frame_width,
                                 enemies[i].frame_height};

            // std::cout << "now enemy img pos is " << enemies[i].current_frame << " and state is " << enemies[i].state << std::endl; // test
            // std::cout << "enemy img size " << enemies[i].frame_width << " " << enemies[i].frame_height << std::endl;               // test
            // std::cout << "enemy state is " << enemies[i].state << std::endl; // test

            SDL_RenderCopy(renderer, enemies[i].texture, &srcRect2, &enemies[i].rect);
        }
        if (enemies[i].state == ENEMY_DIE)
        {
            enemies[i].current_frame = 0;
            enemies[i].state = ENEMY_DIE;
            SDL_Rect srcRect3 = {enemies[i].current_frame * enemies[i].frame_width,
                                 enemies[i].state * enemies[i].frame_height,
                                 enemies[i].frame_width,
                                 enemies[i].frame_height};

            // std::cout << "enemy die img pos" << enemies[i].state << std::endl; // test

            SDL_RenderCopy(renderer, enemies[i].texture, &srcRect3, &enemies[i].rect);
        }
    }

    // 绘制子弹(黄色)
    // TODO ：优化子弹绘制
    bulletTexture = IMG_LoadTexture(renderer, "../assets/Bullet.png");

    SDL_Rect srcRect_bullet, destRect_bullet;

    int frameWidth = 20;  // 假设每帧宽度为20像素
    int frameHeight = 20; // 假设每帧高度为20像素

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (playerBullets[i].active)
        {
            // 设置源矩形（从纹理中裁剪动画帧）
            srcRect_bullet.x = playerBullets[i].frame * frameWidth;
            srcRect_bullet.y = 0;
            srcRect_bullet.w = frameWidth;
            srcRect_bullet.h = frameHeight;

            // 设置目标矩形（子弹在屏幕上的位置）
            destRect_bullet.x = playerBullets[i].rect.x;
            destRect_bullet.y = playerBullets[i].rect.y;
            destRect_bullet.w = playerBullets[i].rect.w;
            destRect_bullet.h = playerBullets[i].rect.h;

            // 渲染当前帧
            SDL_RenderCopy(renderer, bulletTexture, &srcRect_bullet, &destRect_bullet);
        }

        if (enemyBullets[i].active)
        {
            // 设置源矩形（从纹理中裁剪动画帧）
            srcRect_bullet.x = enemyBullets[i].frame * frameWidth;
            srcRect_bullet.y = 0;
            srcRect_bullet.w = frameWidth;
            srcRect_bullet.h = frameHeight;

            // 设置目标矩形（子弹在屏幕上的位置）
            destRect_bullet.x = enemyBullets[i].rect.x;
            destRect_bullet.y = enemyBullets[i].rect.y;
            destRect_bullet.w = enemyBullets[i].rect.w;
            destRect_bullet.h = enemyBullets[i].rect.h;

            // 渲染当前帧
            SDL_RenderCopy(renderer, bulletTexture, &srcRect_bullet, &destRect_bullet);
        }
    }
    /*
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 子弹颜色：黄色
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (playerBullets[i].active)
        {
            SDL_RenderFillRect(renderer, &playerBullets[i].rect);
        }
        if (enemyBullets[i].active)
        {
            SDL_RenderFillRect(renderer, &enemyBullets[i].rect);
        }
    }
    */

    // 显示渲染内容
    SDL_RenderPresent(renderer);
}

bool loadBackground(const char *imagePath)
{
    // 加载背景图片
    backgroundTexture = IMG_LoadTexture(renderer, imagePath);
    if (!backgroundTexture)
    {
        printf("Failed to load background texture! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

void closeSDL(Player *player)
{
    // 释放资源
    if (backgroundTexture)
        SDL_DestroyTexture(backgroundTexture);
    if (player->texture)
        SDL_DestroyTexture(player->texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (win)
        SDL_DestroyWindow(win);
    SDL_Quit();
}