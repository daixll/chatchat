#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

const char* filename = "mjpg.yuv";
int width = 1920; // 图像宽度
int height = 1080; // 图像高度
int frame_size = width * height * 3 / 2; // 每帧数据大小，YUV420格式

int main() {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("无法打开文件 %s\n", filename);
        return 1;
    }

    // 分配缓冲区
    uint8_t* yuv_buffer = (uint8_t*)malloc(frame_size);

    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL 初始化失败: %s\n", SDL_GetError());
        fclose(file);
        free(yuv_buffer);
        return 1;
    }

    // 创建窗口和渲染器
    SDL_Window* window = SDL_CreateWindow("YUV 视频播放器", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, width, height);

    // 等待退出事件
    bool quit = false;
    SDL_Event event;

    while (1) {
        // 读取一帧数据
        fread(yuv_buffer, 1, frame_size, file);
        /*
        if (fread(yuv_buffer, 1, frame_size, file) != frame_size) {
            // 文件结束或者读取错误
            break;
        }
        */

        // 设置纹理数据
        SDL_UpdateYUVTexture(texture, NULL,
                             yuv_buffer, width,
                             yuv_buffer + width * height, width / 2,
                             yuv_buffer + width * height * 5 / 4, width / 2);

        // 渲染纹理到窗口
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // 等待事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_Delay(1000 / 30 * 3 / 2); // 每帧之间等待33毫秒（1000ms / 30fps ≈ 33ms）
        // 文件指针指向开始
        fseek(file, 0, SEEK_SET);
    }

    // 清理资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // 释放缓冲区和关闭文件
    free(yuv_buffer);
    fclose(file);

    return 0;
}
