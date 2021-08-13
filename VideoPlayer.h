#include <vector>
#include <SDL/SDL.h>
extern "C"
{
//#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavutil/version.h>
#include <libswscale/swscale.h>
//#include <libavfilter/avfilter.h>
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
    static int INSTANCIATED;
    int index = 0, width = 1024, height = 768;
    uint64_t timeSinceEpochMillisec();
    std::string get_file_extension(char *c);
    std::vector<std::string> files = {};

    AVFormatContext *format_ctx_input = NULL, *format_ctx_output = NULL;
    AVCodec *codec = NULL, *codec_audio = NULL;
    AVCodecContext *codec_ctx, *codec_ctx_audio;
    AVIOContext *avio_ctx = NULL;
    AVFrame *frame;
    AVPacket pkt;
    SDL_Rect rect;
    AVStream *stream = NULL, *stream_audio = NULL;
    SDL_Surface *screen = NULL;

    bool fill_input_codecs(int &, int &);
    bool isVideoValid(char *);

    void free_memory();
    void recursive_roam(std::string);
    void alloc_contexts();
    void fill_overlay(SDL_Overlay *, int, uint64_t *, struct SwsContext *, int, int, uint64_t);
    void read_inputs(int &);

    VideoPlayer(std::string);

public:
    static VideoPlayer *construct(std::string);
    int loop();
};