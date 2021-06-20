#include <time.h>
#include <vector>
#include <SDL/SDL.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/version.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
}
#include <windows.h>
#include <iostream>
#include <dirent.h>
#include <chrono>

#define NEXT_VIDEO 1
#define PREVIOUS_VIDEO 2
#define QUIT 3

class VideoPlayer
{

private:
    AVFormatContext *format_ctx_input = NULL, *format_ctx_output = NULL;
    AVCodec *codec = NULL, *codec_audio = NULL;
    AVCodecContext *codec_ctx, *codec_ctx_audio;
    AVIOContext *avio_ctx = NULL;
    AVFrame *frame;
    AVPacket pkt;
    SDL_Rect rect;
    AVStream *stream = NULL, *stream_audio = NULL;
    std::vector<std::string> files = {};
    int index = 0, width = 1024, height = 768;
    SDL_Surface *screen = NULL;

    void free_memory();
    void recursive_roam(std::string);
    void alloc_contexts();
    void fill_overlay(SDL_Overlay *, int, uint64_t *, struct SwsContext *, int, int, uint64_t);
    bool fill_input_codecs(int &, int &);
    void read_inputs(int &);

public:
    VideoPlayer(char *);
    bool contains(char *, char *);
    int loop();
    uint64_t timeSinceEpochMillisec();
    std::string get_file_extension(char *c);
};