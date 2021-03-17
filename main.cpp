#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <vector>
#define __STDC_CONSTANT_MACROS
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

using namespace std::chrono;
using namespace std;

#define NEXT_VIDEO 1
#define PREVIOUS_VIDEO 2
#define END 3

void free_memory(AVFormatContext **, AVPacket *, AVFrame **, AVIOContext **, AVFormatContext **, AVCodecContext **, AVCodecContext **);
void recursive_roam(DIR **, const char *, struct dirent **, std::vector<string> *);

uint64_t timeSinceEpochMillisec()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char **argv)
{

    if (!argv[1])
    {
        fprintf(stderr, "Veuillez specifier un chemin de fichier");
        return -1;
    }
    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    srand(time(NULL));
    freopen("/dev/null", "w", stderr);
    freopen("CON", "w", stdout);
    int pause = 0;
    AVFormatContext *format_ctx_input = NULL, *format_ctx_output = NULL;
    AVCodec *codec = NULL, *codec_audio = NULL;
    AVCodecContext *codec_ctx, *codec_ctx_audio;
    AVOutputFormat *output_format = NULL;
    AVFrame *frame;
    AVPacket pkt;
    AVStream *stream = NULL, *stream_audio = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    std::vector<string> vec = {};
    DIR *dir;
    struct dirent *ent;
    const char *parent_folders = argv[1];
    recursive_roam(&dir, parent_folders, &ent, &vec);

    int index = 0;
    if (vec.size() == 0)
    {
        return -10;
    }
    int tab[3000] = {0};
    while (true)
    {

        const char *input_filename = vec[index].c_str();
        string sortie = "videos/" + to_string(index) + ".mp4";
        const char *output_filename = sortie.c_str();
        format_ctx_input = avformat_alloc_context();
        if (avformat_open_input(&format_ctx_input, input_filename, NULL, NULL) != 0)
        {
            fprintf(stderr, "erreur lors de l'ouverture du fichier");
            vec.erase(vec.begin() + index);

            continue;
        }

        codec_ctx = avcodec_alloc_context3(codec);
        codec_ctx_audio = avcodec_alloc_context3(codec_audio);
        auto videoStream = -1;
        auto audioStream = -1;

        int i;
        for (i = 0; i < format_ctx_input->nb_streams; i++)
        {
            if (format_ctx_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStream == -1)
            {
                videoStream = i;
            }
            else if (format_ctx_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStream == -1)
            {
                audioStream = i;
            }
        }
        if (videoStream == -1 || audioStream == -1)
        {
            vec.erase(vec.begin() + index);
            continue;
        }
        //av_dump_format(format_ctx_input, 0,input_filename,0);

        avcodec_copy_context(codec_ctx, format_ctx_input->streams[videoStream]->codec);       // (destination , source)
        avcodec_copy_context(codec_ctx_audio, format_ctx_input->streams[audioStream]->codec); // (destination , source)

        codec = avcodec_find_decoder(codec_ctx->codec_id);

        codec_audio = avcodec_find_decoder(codec_ctx_audio->codec_id);
        //codec_ctx_audio->frame_size = 0;
        if (avcodec_open2(codec_ctx, codec, NULL) != 0)
        {

            fprintf(stderr, "erreur lors de l'ouverture du codec");
            continue;
        }
        av_init_packet(&pkt);

        frame = av_frame_alloc();
        int got_picture = -1;
        int got_audio = -1;

        output_format = av_guess_format(NULL, output_filename, NULL);
        avformat_alloc_output_context2(&format_ctx_output, output_format, NULL, output_filename);

        AVIOContext *avio_ctx = NULL;

        if (avio_open2(&avio_ctx, output_filename, AVIO_FLAG_WRITE, NULL, NULL) < 0)
        {

            fprintf(stderr, "erreur lors de l'ouverture du fichier de SORTIE...");
            return -1;
        }
        format_ctx_output->pb = avio_ctx;
        stream = avformat_new_stream(format_ctx_output, codec);
        stream_audio = avformat_new_stream(format_ctx_output, codec_audio);

        avcodec_parameters_from_context(stream_audio->codecpar, codec_ctx_audio);

        if (format_ctx_output->oformat->flags & AVFMT_GLOBALHEADER)
        {
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        avcodec_parameters_from_context(stream->codecpar, codec_ctx);
        stream->codec->time_base.num = format_ctx_input->streams[videoStream]->time_base.num;
        stream->codec->time_base.den = format_ctx_input->streams[videoStream]->time_base.den;
        avformat_write_header(format_ctx_output, NULL);

        atexit(SDL_Quit);

        SDL_Surface *screen = SDL_SetVideoMode(codec_ctx->width, codec_ctx->height, 0,
                                               SDL_HWSURFACE | SDL_DOUBLEBUF);
        SDL_WM_SetCaption("MP4 Video Player", NULL);
        if (!screen)
        {
            printf("Unable to set video: %s\n", SDL_GetError());
            continue;
        }

        SDL_Overlay *bmp = NULL;
        struct SwsContext *sws_ctx = NULL;

        bmp = SDL_CreateYUVOverlay(codec_ctx->width, codec_ctx->height,
                                   SDL_YV12_OVERLAY, screen);

        sws_ctx = sws_getContext(codec_ctx->width,
                                 codec_ctx->height,
                                 AV_PIX_FMT_YUV420P,
                                 codec_ctx->width,
                                 codec_ctx->height,
                                 AV_PIX_FMT_YUV420P,
                                 SWS_BILINEAR,
                                 NULL,
                                 NULL,
                                 NULL);

        SDL_Flip(screen);

        int quit = 0;
        uint64_t last = NULL;

        uint64_t lastDisplayed = NULL;
        while (av_read_frame(format_ctx_input, &pkt) == 0 && quit == 0)
        {
            SDL_PollEvent(&event);
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:

                    index = rand() % (vec.size() - 1 - 0 + 1) + 0;

                    quit = NEXT_VIDEO;
                    break;
                case SDLK_RIGHT:
                    quit = NEXT_VIDEO;

                    break;
                case SDLK_LEFT:
                    quit = PREVIOUS_VIDEO;

                    break;

                default:
                    break;
                }
                break;
            case SDL_QUIT:
                SDL_Quit();
                break;

            default:
                break;
            }
            if (quit == NEXT_VIDEO || quit == PREVIOUS_VIDEO)
                break;

            if (pkt.stream_index == videoStream)
            {
                last = timeSinceEpochMillisec();
                avcodec_decode_video2(codec_ctx, frame, &got_picture, &pkt);
                const double frameDuration = (1000.0 / (format_ctx_input->streams[videoStream]->r_frame_rate.num / format_ctx_input->streams[videoStream]->r_frame_rate.den));
                uint64_t delay = timeSinceEpochMillisec() - last;
                if (frameDuration > delay)
                    Sleep(frameDuration - delay);
                SDL_Rect rect;

                if (got_picture == 1)
                {
                    SDL_LockYUVOverlay(bmp);
                    AVPicture pict;
                    pict.data[0] = bmp->pixels[0];
                    pict.data[1] = bmp->pixels[2];
                    pict.data[2] = bmp->pixels[1];

                    pict.linesize[0] = bmp->pitches[0];
                    pict.linesize[1] = bmp->pitches[2];
                    pict.linesize[2] = bmp->pitches[1];

                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data,
                              frame->linesize, 0, codec_ctx->height,
                              pict.data, pict.linesize);

                    SDL_UnlockYUVOverlay(bmp);
                    rect.x = 0;
                    rect.y = 0;
                    rect.w = codec_ctx->width;
                    rect.h = codec_ctx->height;

                    SDL_DisplayYUVOverlay(bmp, &rect);
                    if (lastDisplayed != NULL)
                        cout << 1000.0 / (timeSinceEpochMillisec() - lastDisplayed) << " fps" << endl;

                    lastDisplayed = timeSinceEpochMillisec();
                }
                if (pkt.pts != AV_NOPTS_VALUE)
                    pkt.pts = av_rescale_q(pkt.pts, stream->codec->time_base, stream->time_base);
                if (pkt.dts != AV_NOPTS_VALUE)
                    pkt.dts = av_rescale_q(pkt.dts, stream->codec->time_base, stream->time_base);

                av_interleaved_write_frame(format_ctx_output, &pkt);
            }
            else if (pkt.stream_index == audioStream)
            {
                av_interleaved_write_frame(format_ctx_output, &pkt);
            }

            av_free_packet(&pkt);
            av_frame_unref(frame);
        }

        if (quit == NEXT_VIDEO || quit == 0)
        {
            cout << "next" << quit << endl;

            if (index < vec.size() - 1)
            {
                index++;
            }
            else
            {
                index = 0;
            }
        }
        if (quit == PREVIOUS_VIDEO)
        {
            cout << "previous" << quit << endl;

            if (index > 0)
            {
                index--;
            }
            else
            {
                index = vec.size() - 1;
            }
        }

        SDL_FreeYUVOverlay(bmp);
        av_write_trailer(format_ctx_output);
        free_memory(&format_ctx_output, &pkt, &frame, &avio_ctx, &format_ctx_input, &codec_ctx, &codec_ctx_audio);
        if (quit == END)
            break;
    }

    return 0;
}
void recursive_roam(DIR **dir, const char *parent, struct dirent **ent, std::vector<string> *vec)
{

    if ((*dir = opendir(parent)) != NULL)
    {
        while ((*ent = readdir(*dir)) != NULL)
        {
            string p = parent;
            string s = (*ent)->d_name;

            if (s[s.size() - 1] != '.' && (*ent)->d_type == 16)
            {
                std::cout << "Dossier: " + s << endl;
                DIR *d;
                string parent_fold = parent;
                struct dirent *ent2;

                parent_fold += s + '/';

                recursive_roam(&d, parent_fold.c_str(), &ent2, &(*vec));
            }
            else if (s[s.size() - 1] != '.' && s.find(".jpg", 0) == string::npos && s.find(".png", 0) == string::npos)
            {
                std::cout << "\tfichier mp4: " + s << endl;
            }
            else
                continue;

            p = p + s;

            vec->push_back(p);
        }
        closedir(*dir);
    }
}
void free_memory(AVFormatContext **format_ctx_output, AVPacket *pkt, AVFrame **frame, AVIOContext **avio_ctx, AVFormatContext **format_ctx_input, AVCodecContext **codec_ctx, AVCodecContext **codec_ctx_audio)
{
    avcodec_free_context(&(*codec_ctx));
    avcodec_free_context(&(*codec_ctx_audio));
    av_packet_unref(pkt);
    av_frame_unref(*frame);
    av_frame_free(frame);
    avio_close(*avio_ctx);
    avformat_free_context(*format_ctx_output);
    avformat_free_context(*format_ctx_input);
}