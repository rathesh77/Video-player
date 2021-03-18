#include <time.h>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
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

using namespace std;

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
    AVStream *stream = NULL, *stream_audio = NULL;
    std::vector<string> files = {};
    int index = 0;
    
    void free_memory();
    void recursive_roam(const char *);
    void alloc_contexts();
    void fill_overlay(SDL_Overlay *, int, uint64_t *, struct SwsContext *, int, int, uint64_t);

public:
    VideoPlayer(char *);
    bool contains(char *, char *);
    bool fill_input_codecs(int *, int *);
    void read_inputs(int *);
    int loop();
    uint64_t timeSinceEpochMillisec();
    string get_file_extension(char *c);
};