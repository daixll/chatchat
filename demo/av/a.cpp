extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <SDL2/SDL.h>

#define ERRIF(flg, ...) do { \
    if (flg == 1){ \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\nerrno[%d]: %s\n", errno, strerror(errno)); \
        exit(1); \
    } \
} while (0)

int main(int argc, char* argv[]) {
    avdevice_register_all(); // 注册所有输入输出设备

    AVFormatContext* pFormatCtx = avformat_alloc_context();         // 格式上下文
    const AVInputFormat* iformat = av_find_input_format("v4l2");    // 查找视频输入格式
    ERRIF(!iformat, "Cannot find input format %s", "v4l2");         // 如果找不到输入格式，打印错误信息

    AVDictionary* options = NULL;                                   // 定义字典用于传递选项
    av_dict_set(&options, "input_format", "mjpeg", 0);              // 输入流格式为 mjpeg
    av_dict_set(&options, "video_size", "1920x1080", 0);            // 视频尺寸为 1920x1080
    av_dict_set(&options, "framerate", "30", 0);                    // 帧率为 30

    // 打开视频设备
    ERRIF(avformat_open_input(&pFormatCtx, "/dev/video0", iformat, &options) != 0, "Cannot open video device");

    // 打开视频流
    ERRIF(avformat_find_stream_info(pFormatCtx, NULL) < 0, "Cannot find stream information");

    const AVCodec* pCodec = NULL;           // 定义编解码器
    AVCodecParameters* pCodecParams = NULL; // 定义编解码器参数
    int videoStreamIndex = -1;              // 视频流索引初始化为-1

    for(int i=0; i < pFormatCtx->nb_streams; i++)                               // 遍历所有流
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){ // 如果是视频流
            videoStreamIndex = i;                                               // 设置视频流索引
            pCodecParams = pFormatCtx->streams[i]->codecpar;                    // 获取编解码器参数
            pCodec = avcodec_find_decoder(pCodecParams->codec_id);              // 查找编解码器
            break; // 退出循环
        }

    // 如果找不到视频流，打印错误信息
    ERRIF(videoStreamIndex == -1, "Cannot find video stream");

    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);     // 分配编解码器上下文
    avcodec_parameters_to_context(pCodecCtx, pCodecParams);         // 从编解码器参数初始化编解码器上下文

    // 如果无法打开编解码器，打印错误信息
    ERRIF(avcodec_open2(pCodecCtx, pCodec, NULL) < 0, "Cannot open codec");

    AVFrame* pFrame = av_frame_alloc(); // 分配视频帧
    AVPacket packet;                    // 定义包

    SDL_Init(SDL_INIT_VIDEO); // 初始化SDL视频
    SDL_Window* screen = SDL_CreateWindow("FFmpeg SDL Video Player", // 创建SDL窗口
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1920, 1080,
                                          SDL_WINDOW_OPENGL);

    SDL_Renderer* renderer = SDL_CreateRenderer(screen, -1, 0); // 创建渲染器

    SDL_Texture* texture = SDL_CreateTexture(renderer, // 创建纹理
                                             SDL_PIXELFORMAT_YV12,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             pCodecCtx->width,
                                             pCodecCtx->height);

    struct SwsContext* sws_ctx = sws_getContext(pCodecCtx->width, // 创建图像转换上下文
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    while (av_read_frame(pFormatCtx, &packet) >= 0) { // 读取帧
        if (packet.stream_index == videoStreamIndex) { // 如果是视频帧
            if (avcodec_send_packet(pCodecCtx, &packet) == 0) { // 发送包到编解码器
                while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) { // 接收帧
                    AVFrame* pFrameYUV = av_frame_alloc(); // 分配YUV帧
                    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, // 获取图像缓冲区大小
                                                            pCodecCtx->width,
                                                            pCodecCtx->height, 1);
                    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t)); // 分配缓冲区
                    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, buffer, // 填充图像数据
                                         AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                         pCodecCtx->height, 1);

                    sws_scale(sws_ctx, (uint8_t const* const*)pFrame->data, // 缩放图像
                              pFrame->linesize, 0, pCodecCtx->height,
                              pFrameYUV->data, pFrameYUV->linesize);
                    
                    SDL_UpdateYUVTexture(texture, NULL, // 更新纹理
                                         pFrameYUV->data[0], pFrameYUV->linesize[0],
                                         pFrameYUV->data[1], pFrameYUV->linesize[1],
                                         pFrameYUV->data[2], pFrameYUV->linesize[2]);
                    

                    SDL_RenderClear(renderer); // 清空渲染器
                    
                    SDL_RenderCopy(renderer, texture, NULL, NULL); // 渲染纹理
                    
                    SDL_RenderPresent(renderer); // 显示渲染结果

                    // 保存帧到文件
                    FILE* file = fopen("mjpg.yuv", "wb+");
                    ERRIF(!file, "Cannot open file");
                    fwrite(pFrameYUV->data[0], 1, pCodecCtx->width * pCodecCtx->height, file); // 写入Y分量
                    fwrite(pFrameYUV->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4, file); // 写入U分量
                    fwrite(pFrameYUV->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, file); // 写入V分量
                    fclose(file);

                    av_free(buffer); // 释放缓冲区
                    av_frame_free(&pFrameYUV); // 释放YUV帧
                }
            }
        }
        av_packet_unref(&packet); // 释放包
    }

    sws_freeContext(sws_ctx); // 释放图像转换上下文
    
    SDL_DestroyTexture(texture); // 销毁纹理
    
    SDL_DestroyRenderer(renderer); // 销毁渲染器
    SDL_DestroyWindow(screen); // 销毁窗口
    SDL_Quit(); // 退出SDL

    avcodec_free_context(&pCodecCtx); // 释放编解码器上下文
    avformat_close_input(&pFormatCtx); // 关闭输入
    av_frame_free(&pFrame); // 释放视频帧
    av_dict_free(&options); // 释放字典

    return 0; // 返回0，表示成功
}
